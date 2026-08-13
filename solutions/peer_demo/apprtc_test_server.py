from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.parse import parse_qs, urlparse
from urllib.request import Request, urlopen

import argparse


BASE_DIR = Path(__file__).resolve().parent
ALLOWED_HOST = "webrtc.espressif.com"


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(BASE_DIR), **kwargs)

    def do_POST(self):
        if self.path.startswith("/proxy?"):
            self._proxy()
            return
        super().do_POST()

    def do_GET(self):
        if self.path.startswith("/proxy?"):
            self._proxy()
            return
        super().do_GET()

    def _proxy(self):
        parsed = urlparse(self.path)
        query = parse_qs(parsed.query)
        target = query.get("url", [None])[0]
        if not target:
            self.send_error(400, "missing url")
            return

        target_parsed = urlparse(target)
        if target_parsed.scheme != "https" or target_parsed.hostname != ALLOWED_HOST:
            self.send_error(403, "target not allowed")
            return

        body = b""
        if self.command in {"POST", "PUT", "PATCH"}:
            length = int(self.headers.get("Content-Length", "0"))
            body = self.rfile.read(length) if length else b""

        headers = {
            "User-Agent": "peer-demo-minimal-test",
            "Accept-Encoding": "identity",
        }
        content_type = self.headers.get("Content-Type")
        if content_type:
            headers["Content-Type"] = content_type

        request = Request(target, data=body if self.command != "GET" else None, headers=headers, method=self.command)

        try:
            with urlopen(request, timeout=20) as resp:
                payload = resp.read()
                self.send_response(resp.status)
                self.send_header("Content-Type", resp.headers.get("Content-Type", "text/plain; charset=utf-8"))
                self.send_header("Content-Length", str(len(payload)))
                self.send_header("Cache-Control", "no-store")
                self.end_headers()
                self.wfile.write(payload)
        except HTTPError as error:
            payload = error.read()
            self.send_response(error.code)
            self.send_header("Content-Type", error.headers.get("Content-Type", "text/plain; charset=utf-8"))
            self.send_header("Content-Length", str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)
        except URLError as error:
            payload = str(error).encode()
            self.send_response(502)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", default=8000, type=int)
    args = parser.parse_args()

    server = ThreadingHTTPServer((args.host, args.port), Handler)
    print(f"Serving {BASE_DIR} on http://{args.host}:{args.port}")
    server.serve_forever()


if __name__ == "__main__":
    main()
