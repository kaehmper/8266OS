import http.server
import socketserver
import re

PORT = 8000

# Extract the HTML from WebPages.h
html_content = ""
with open("NumberStationOS/WebPages.h", "r") as f:
    content = f.read()
    match = re.search(r'const char PAGE_INDEX\[\] PROGMEM = R"=====\((.*?)\)=====";', content, re.DOTALL)
    if match:
        html_content = match.group(1)

with open("/tmp/index.html", "w") as f:
    f.write(html_content)

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory="/tmp", **kwargs)

with socketserver.TCPServer(("", PORT), Handler) as httpd:
    print(f"Serving at port {PORT}")
    httpd.serve_forever()
