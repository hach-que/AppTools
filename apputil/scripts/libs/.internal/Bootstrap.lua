-- Define our package loader.

package.path = "./" .. _APPUTIL_LIB_PATH .. ".internal/?.lua"
require("ModuleLoader")
