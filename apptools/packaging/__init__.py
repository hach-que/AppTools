#!/usr/bin/env python

from apptools.native import fs
import os
import subprocess
import sys
import shutil
import tarfile
from pyparsing import Literal, Or, Word, printables, alphanums, alphas, LineEnd, Optional

CONFIG_PREP_ROOT = None #"/build/prep"
CONFIG_APP_ROOT = None #"/data/data/kevinboone.androidterm/kbox/usr"

__all__ = [ "parse_config", "Package", "CONFIG_PREP_ROOT", "CONFIG_APP_ROOT" ]

class ConfigureProvider():
    def __init__(self, pkgname):
        # Package name
        self._name = pkgname

        # Define include paths
        self._include_paths = list()
        self._include_paths.append("/build/toolchain/include")

        # Define linker paths
        self._linker_paths = list()

        # Define linker libraries
        self._linker_libs = list()

        # Define configure parameters
        self._configure_params = list()

        # Define hacks
        self._hacks = list()

        # Define default CPU
        self._cpu = "arm-unknown-linux-gnueabi"

        # Define make targets
        self._make_targets = list()

        # Define make install targets
        self._make_install_targets = list()

        # Define on-failure, on-success and on-recover commands
        self._on_failure_commands = list()
        self._on_success_commands = list()
        self._on_recover_commands = list() # list of tuples

    def accept(self, args):
        if (args[0] == "include" and args[1] == "headers"):
            self._include_paths.append(args[2])
        elif (args[0] == "include" and args[1] == "libraries"):
            self._linker_paths.append(args[2])
        elif (args[0] == "link"):
            self._linker_libs.append(args[1])
        elif (args[0] == "option" and args[1] == "configure" and len(args) == 3):
            self._configure_params.append(args[2])
        elif (args[0] == "option" and args[1] == "hack" and len(args) == 3):
            self._hacks.append(args[2].lower())
        elif (args[0] == "option" and args[1] == "cpu" and len(args) == 3):
            self._cpu = args[2]
        elif (args[0] == "option" and args[1] == "force-android" and len(args) == 2):
            self._cpu = "arm-linux-androideabi"
            self._include_paths.append("/build/toolchain/arm-linux-androideabi/include/c++/4.6/")
            self._include_paths.append("/build/toolchain/arm-linux-androideabi/include/c++/4.6/arm-linux-androideabi/")
            print "warning: Using Android NDK toolchain.  This is probably not what you want!"
            print "         Most standard Linux software will not build against Android's"
            print "         limited C library and you will get compile errors.  Only use this"
            print "         option when targeting native Android code against the Android NDK."
        elif (args[0] == "option" and args[1] == "make-target" and len(args) == 3):
            self._make_targets.append(args[2])
        elif (args[0] == "option" and args[1] == "make-install-target" and len(args) == 3):
            self._make_install_targets.append(args[2])
        elif (args[0] == "option" and args[1] == "on-failure" and len(args) == 3):
            self._on_failure_commands.append(args[2])
        elif (args[0] == "option" and args[1] == "on-success" and len(args) == 3):
            self._on_success_commands.append(args[2])
        elif (args[0] == "option" and args[1].startswith("on-recover-") and len(args) == 3):
            try:
                self._on_recover_commands.append((int(args[1][len("on-recover-"):]), args[2]))
            except e:
                raise Exception("invalid on-recover entry; must have numeric suffix")
        else:
            raise Exception("unknown entry in configuration file: " + str(args))

    def _execute(self, args):
        p = subprocess.Popen(args, env=self._env)
        p.wait()
        if (p.returncode != 0):
            return False
        else:
            return True

    def _execute_global(self, args):
        p = subprocess.Popen(args)
        p.wait()
        if (p.returncode != 0):
            return False
        else:
            return True

    def _on_failure(self):
        for i in self._on_failure_commands:
            self._execute_global(["sh", "-c", i])

    def _on_success(self):
        for i in self._on_success_commands:
            self._execute_global(["sh", "-c", i])
    
    def _on_recover(self, count):
        executed = False
        for i in self._on_recover_commands:
            if (i[0] == count):
                self._execute_global(["sh", "-c", i[1]])
                executed = True
        return executed

    def _ensure_execute(self, args):
        passed = False
        while not passed:
            passed = self._execute(args)
            if (not passed):
                if (not self._on_recover(self._attempt_count)):
                    # There are no more failure commands to execute.
                    self._on_failure()
                    return False
                self._attempt_count += 1
            else:
                return True

    def _fail(self):
        print "error: Build failed; no more recovery commands to execute"
        sys.exit(1)

    def build(self, config):
        # Create environment variables
        LDFLAGS = ""
        CFLAGS = "-Wno-error -g -O2 -nostdinc "
        CCFLAGS = "-Wno-error -g -O2 -nostdinc "
        CXXFLAGS = "-Wno-error -g -O2 -nostdinc -nostdinc++ "
        CPPFLAGS = "-Wno-error "
        for i in self._linker_paths:
            LDFLAGS += "-L" + i + " "
        for i in self._linker_libs:
            LDFLAGS += "-l" + i + " "
        for i in self._include_paths:
            CFLAGS += "-I" + i + " "
            CCFLAGS += "-I" + i + " "
            CXXFLAGS += "-I" + i + " "
            CPPFLAGS += "-I" + i + " "

        # Apply hacks
        for i in self._hacks:
            if (i == "localeconv"):
                CFLAGS += "-D__HACK_BUILTIN_LOCALECONV"
                CCFLAGS += "-D__HACK_BUILTIN_LOCALECONV"
                CXXFLAGS += "-D__HACK_BUILTIN_LOCALECONV"
                CPPFLAGS += "-D__HACK_BUILTIN_LOCALECONV"
            else:
                raise Exception("unknown hack '" + i + "' specified in config file")

        # Create enviroment dictionary
        self._env = {
            "LDFLAGS": LDFLAGS,
            "CFLAGS": CFLAGS,
            "CCFLAGS": CCFLAGS,
            "CXXFLAGS": CXXFLAGS,
            "CPPFLAGS": CPPFLAGS,
            "PATH": "/build/toolchain/bin/",
        }

        # Generate configure parameters.
        configure = list()
        configure.append("../src/configure")
        configure.append("--prefix=" + CONFIG_APP_ROOT)
        configure.append("--build=x86_64-unknown-linux-gnu")
        configure.append("--host=" + self._cpu)
        configure.append("--target=" + self._cpu)
        for i in self._configure_params:
            configure.append(i)

        # Execute build.
        self._attempt_count = 1
        if (not config["no-configure"]):
            if (not self._ensure_execute(configure)):
                self._fail()
        if (not config["no-substep-build"]):
            if (len(self._make_targets) == 0):
                if (not self._ensure_execute(["make"])):
                    self._fail()
            else:
                for i in self._make_targets:
                    if (not self._ensure_execute(["make", i])):
                        self._fail()
        if (not config["no-substep-install"]):
            if (len(self._make_install_targets) == 0):
                if (not self._ensure_execute(["make", "install", "DESTDIR=/build/prep/" + self._name + "/install"])):
                    self._fail()
            else:
                for i in self._make_install_targets:
                    if (not self._ensure_execute(["make", i, "DESTDIR=/build/prep/" + self._name + "/install"])):
                        self._fail()
        self._on_success()

    def continue_(self, config):
        # Build continuation not supported.
        return False

    def get_output(self):
        return "/build/prep/" + self._name + "/install/" + CONFIG_APP_ROOT

class CMakeProvider():
    def __init__(self, pkgname):
        # Package name
        self._name = pkgname

        # Define include paths
        self._include_paths = list()
        #self._include_paths.append("/build/toolchain/arm-linux-androideabi/include/c++/4.6/")
        #self._include_paths.append("/build/toolchain/arm-linux-androideabi/include/c++/4.6/arm-linux-androideabi/")

        # Define linker paths
        self._linker_paths = list()

        # Define linker libraries
        self._linker_libs = list()

        # Define configure parameters
        self._configure_params = list()

    def accept(self, args):
        if (args[0] == "include" and args[1] == "headers"):
            self._include_paths.append(args[2])
        elif (args[0] == "include" and args[1] == "libraries"):
            self._linker_paths.append(args[2])
        elif (args[0] == "link"):
            self._linker_libs.append(args[1])
        elif (args[0] == "option" and args[1] == "cmake" and len(args) == 3):
            self._configure_params.append(args[2])
        else:
            raise Exception("unknown entry in configuration file: " + str(args))

    def _execute(self, args):
        p = subprocess.Popen(args, env=self._env)
        p.wait()
        if (p.returncode != 0):
            sys.exit(1)

    def build(self, config):
        # Create environment variables
        LDFLAGS = ""
        CFLAGS = "-Wno-error "
        CCFLAGS = "-Wno-error "
        CXXFLAGS = "-Wno-error "
        CPPFLAGS = "-Wno-error "
        for i in self._linker_paths:
            LDFLAGS += "-L" + i + " "
        for i in self._linker_libs:
            LDFLAGS += "-l" + i + " "
        for i in self._include_paths:
            CFLAGS += "-I" + i + " "
            CCFLAGS += "-I" + i + " "
            CXXFLAGS += "-I" + i + " "
            CPPFLAGS += "-I" + i + " "

        # Create enviroment dictionary
        self._env = {
            "LDFLAGS": LDFLAGS,
            "CFLAGS": CFLAGS,
            "CCFLAGS": CCFLAGS,
            "CXXFLAGS": CXXFLAGS,
            "CPPFLAGS": CPPFLAGS,
            "PATH": "/build/toolchain/bin:" + os.environ["PATH"],
        }

        # Generate configure parameters.
        cmake = list()
        cmake.append("cmake")
        cmake.append("-DCMAKE_SYSTEM_NAME=Linux")
        cmake.append("-DCMAKE_SYSTEM_PROCESSOR=ARM7")
        cmake.append("-DCMAKE_C_COMPILER=arm-linux-androideabi-gcc")
        cmake.append("-DCMAKE_FIND_ROOT_PATH=/build/toolchain/")
        cmake.append("-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY")
        cmake.append("-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY")
        cmake.append("-DCMAKE_INSTALL_PREFIX=/build/prep/" + self._name + "/install")
        for i in self._configure_params:
            cmake.append("-D" + i)
        cmake.append("/build/prep/" + self._name + "/src")

        # Execute build.
        if (not config["no-configure"]):
            self._execute(cmake)
        if (not config["no-substep-build"]):
            self._execute(["make"])

    def continue_(self, config):
        # Build continuation not supported.
        return False

    def get_output(self):
        return "/build/prep/" + self._name + "/install"

class AppToolsProvider():
    def __init__(self, name):
        self._name = name
    
    def package(self, config, file_path):
        if (os.path.exists(self._name + ".afs")):
            print "Removing old AppTools package..."
            os.unlink(self._name + ".afs")
        print "Creating AppTools package..."
        fs.Package.create(self._name + ".afs", self._name, "Unversioned", "", "")
        pkg = fs.Package(self._name + ".afs")
        for root, folders, files in os.walk(file_path):
            for i in folders:
                pkg.mkdir(os.path.join(root, i)[len(file_path)+1:], 0755)
            for i in files:
                # We don't support reading or writing to files yet in the API.
                pass

class TarProvider():
    def __init__(self, name):
        self._name = name

    def package(self, config, file_path):
        if (os.path.exists(self._name + ".tar.gz")):
            print "Removing old tar.gz package..."
            os.unlink(self._name + ".tar.gz")
        print "Creating tar.gz package..."
        count = 0
        tar = tarfile.open(self._name + ".tar.gz", "w:gz")
        for root, folders, files in os.walk(file_path):
            for i in folders:
                tar.add(os.path.join(root, i), arcname=os.path.join(root, i)[len(file_path):], recursive=False)
                count += 1
                self.print_status(count)
            for i in files:
                tar.add(os.path.join(root, i), arcname=os.path.join(root, i)[len(file_path):], recursive=False)
                count += 1
                self.print_status(count)
        tar.close()

    def print_status(self, count):
        if (count < 1000 and count % 50 == 0):
            print ".. " + str(count) + " entries added"
        elif (count % 1000 == 0):
            print ".. " + str(count) + " entries added"

class AllProvider():
    def __init__(self, name):
        self._name = name

    def package(self, config, file_path):
        for i in package_providers:
            if (i == "all"):
                continue
            pp = package_providers[i](self._name)
            pp.package(config, file_path)

build_providers = {
    "configure": ConfigureProvider,
    "cmake": CMakeProvider
}

package_providers = {
    "apptools": AppToolsProvider,
    "tar": TarProvider,
    "all": AllProvider
}

def parse_config(filename, pkgname):
    result = {}
    result["include_paths"] = list()
    result["linker_paths"] = list()
    result["linker_libs"] = list()

    grammar = ( Literal("type") + (
                  Literal("configure") |
                  Literal("cmake") ) ) | \
              ( Literal("include") + (
                  Literal("headers") | \
                  Literal("libraries") ) + Word(printables + " ") + LineEnd() ) | \
              ( Literal("link") + Word(printables + " ") + LineEnd() ) | \
              ( Literal("option") + Word(alphanums + "-") + Optional(Word(printables + " ")) + LineEnd() )

    current_type = None

    for l in open(filename):
        l = l.strip()
        if (len(l) == 0):
            continue # Ignore blank lines
        if (l[0] == "#"):
            continue # Ignore lines starting with a hash
        if (CONFIG_APP_ROOT == None or CONFIG_PREP_ROOT == None):
            raise Exception("set the CONFIG_APP_ROOT and CONFIG_PREP_ROOT settings before parsing configuration files")
        l = l.replace("%ROOT%", CONFIG_APP_ROOT)
        l = l.replace("%PREP%", CONFIG_PREP_ROOT)
        inp = grammar.parseString(l)
        if (inp[0] == "type" and current_type == None):
            current_type = build_providers[inp[1]](pkgname)
        elif (inp[0] == "type" and current_type != None):
            raise Exception("type can not be specified twice")
        elif (current_type == None):
            raise Exception("type must be specified before any other directives")
        else:
            current_type.accept(inp)

    return current_type

class Package():
    def __init__(self, name):
        # Package name
        self._name = str(name)

    def build(self, config):
        if (CONFIG_APP_ROOT == None or CONFIG_PREP_ROOT == None):
            raise Exception("set the CONFIG_APP_ROOT and CONFIG_PREP_ROOT settings before building packages")
        if (not config["no-clean"]):
            if (not config["no-configure"]):
                if (os.path.exists(CONFIG_PREP_ROOT + "/" + self._name + "/build")):
                    print "Cleaning out existing build folder..."
                    shutil.rmtree(CONFIG_PREP_ROOT + "/" + self._name + "/build")
            if (os.path.exists(CONFIG_PREP_ROOT + "/" + self._name + "/install")):
                print "Cleaning out existing install folder..."
                shutil.rmtree(CONFIG_PREP_ROOT + "/" + self._name + "/install")
        if (not os.path.exists(CONFIG_PREP_ROOT + "/" + self._name + "/build")):
            os.mkdir(CONFIG_PREP_ROOT + "/" + self._name + "/build")
        if (not os.path.exists(CONFIG_PREP_ROOT + "/" + self._name + "/install")):
            os.mkdir(CONFIG_PREP_ROOT + "/" + self._name + "/install")
        print "Changing to build directory..."
        os.chdir(CONFIG_PREP_ROOT + "/" + self._name + "/build")
        provider = parse_config(CONFIG_PREP_ROOT + "/" + self._name + "/config", self._name)
        provider.build(config)

    def continue_(self, config):
        if (CONFIG_APP_ROOT == None or CONFIG_PREP_ROOT == None):
            raise Exception("set the CONFIG_APP_ROOT and CONFIG_PREP_ROOT settings before continuing a build")
        if (not os.path.exists(CONFIG_PREP_ROOT + "/" + self._name + "/build")):
            os.mkdir(CONFIG_PREP_ROOT + "/" + self._name + "/build")
        if (not os.path.exists(CONFIG_PREP_ROOT + "/" + self._name + "/install")):
            os.mkdir(CONFIG_PREP_ROOT + "/" + self._name + "/install")
        print "Changing to build directory..."
        os.chdir(CONFIG_PREP_ROOT + "/" + self._name + "/build")
        provider = parse_config(CONFIG_PREP_ROOT + "/" + self._name + "/config", self._name)
        if (not provider.continue_(config)):
            print "error: Build continuation not supported for this build system."
            return False
        return True

    def package(self, config):
        if (CONFIG_APP_ROOT == None or CONFIG_PREP_ROOT == None):
            raise Exception("set the CONFIG_APP_ROOT and CONFIG_PREP_ROOT settings before packaging software")
        if (not os.path.exists(CONFIG_PREP_ROOT + "/" + self._name + "/output")):
            os.mkdir(CONFIG_PREP_ROOT + "/" + self._name + "/output")
        print "Changing to output directory..."
        os.chdir(CONFIG_PREP_ROOT + "/" + self._name + "/output")
        build = parse_config(CONFIG_PREP_ROOT + "/" + self._name + "/config", self._name)
        output = package_providers[config["output"]](self._name)
        output.package(config, build.get_output())
