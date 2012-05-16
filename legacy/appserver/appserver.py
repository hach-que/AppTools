import os
import sys
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
from urlparse import urlparse
import handlers
import appserverdb
import sqlite3

class AppServer(BaseHTTPRequestHandler):
    def __init__(self, request, client_address, server):
        BaseHTTPRequestHandler.__init__(self, request, client_address, server)
    
    def do_GET(self):
        if (self.server.db == None):
            self.server.db = sqlite3.connect(":memory:");
            appserverdb.generate(self.server.db); # Generates the tables
            appserverdb.populate("./packages/"); # Populates the database cache
                                                 # from the .afs packages stored
                                                 # in the provided path.
    
        p = urlparse(self.path);
        if p.path[1:] in handlers.supported:
            try:
                handlers.supported[p.path[1:]].main(
                    {url=p, request=self, out=self.wfile, db=self.server.db}
                    );
            except:
                print "There is no main() function in the '" + p.path[1:] + "' handler."
                self.send_response(404);
                self.end_headers();
        else:
            print "There is no handler for '" + p.path[1:] + "'."
            self.send_response(404);
            self.end_headers();

def main():
    try:
        print "Starting AppServer..."
        srv = HTTPServer(('',80), AppServer)
        srv.serve_forever()
    except KeyboardInterrupt:
        print "Terminating server..."
        srv.socket.close()

if __name__ == "__main__":
    main()