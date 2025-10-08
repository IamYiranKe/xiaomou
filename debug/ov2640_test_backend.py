import asyncio
import json
import uuid

from aiohttp import web


VERSION = "debug-backend aiohttp v2"


def log(msg: str) -> None:
    print(f"[backend] {VERSION}: {msg}")


HOST_IP = "192.168.1.18"
HTTP_PORT = 8000
SESSION_ID = str(uuid.uuid4())
CAPTURE_PATH = "capture.jpg"


async def ota_handler(request: web.Request) -> web.StreamResponse:
    log(f"OTA request from {request.remote}")
    return web.json_response({
        "firmware": {"version": "local"},
        "websocket": {
            "url": f"ws://{HOST_IP}:{HTTP_PORT}/ws",
            "version": 3,
            "token": ""
        }
    })


async def upload_handler(request: web.Request) -> web.StreamResponse:
    log(f"Upload request from {request.remote}")
    reader = await request.multipart()
    question = None
    image_bytes = bytearray()

    async for part in reader:
        if part.name == "question":
            question = await part.text()
            log(f"Received question: {question}")
        elif part.name == "file":
            while True:
                chunk = await part.read_chunk()
                if not chunk:
                    break
                image_bytes.extend(chunk)
            log(f"Received image bytes: {len(image_bytes)}")

    with open(CAPTURE_PATH, "wb") as file:
        file.write(image_bytes)
    log(f"Saved capture to {CAPTURE_PATH}")

    return web.json_response({"success": True, "question": question or ""})


async def http_server() -> None:
    log("Starting HTTP server")
    app = web.Application()
    app.router.add_route("GET", "/ota.json", ota_handler)
    app.router.add_route("POST", "/ota.json", ota_handler)
    app.router.add_post("/upload", upload_handler)
    app.router.add_get("/ws", websocket_handler)

    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, "0.0.0.0", HTTP_PORT)
    await site.start()
    log(f"HTTP server bound to 0.0.0.0:{HTTP_PORT}")
    await asyncio.Future()


async def websocket_handler(request: web.Request) -> web.WebSocketResponse:
    ws = web.WebSocketResponse()
    await ws.prepare(request)

    log("WebSocket client connected")
    await ws.send_json({
        "type": "hello",
        "transport": "websocket",
        "session_id": SESSION_ID,
        "audio_params": {
            "format": "opus",
            "sample_rate": 16000
        }
    })
    log("WS sent hello greeting")

    try:
        msg = await ws.receive()
    except Exception as exc:
        log(f"WS failed waiting for client hello: {exc}")
        await ws.close()
        return ws

    if msg.type == web.WSMsgType.TEXT:
        try:
            hello = json.loads(msg.data)
            log(f"WS recv hello: {hello}")
        except Exception as exc:
            log(f"WS recv invalid hello: {exc}")
    else:
        log(f"WS recv unexpected message type: {msg.type}")

    await asyncio.sleep(0.5)
    await ws.send_json({
        "type": "mcp",
        "payload": {
            "jsonrpc": "2.0",
            "method": "initialize",
            "params": {
                "capabilities": {
                    "vision": {
                        "url": f"http://{HOST_IP}:{HTTP_PORT}/upload",
                        "token": ""
                    }
                }
            },
            "id": 1
        }
    })
    log("WS sent initialize request")

    async for msg in ws:
        if msg.type != web.WSMsgType.TEXT:
            log(f"WS recv non-text message: {msg.type}")
            continue
        try:
            message = json.loads(msg.data)
        except Exception as exc:
            log(f"WS recv non-JSON message: {exc}")
            continue
        log(f"WS recv: {message}")

        if message.get("type") != "mcp":
            continue

        payload = message.get("payload", {})
        if payload.get("id") == 1 and "result" in payload:
            log("Initialization acknowledged; sending take_photo command")
            await ws.send_json({
                "type": "mcp",
                "payload": {
                    "jsonrpc": "2.0",
                    "method": "tools/call",
                    "params": {
                        "name": "self.camera.take_photo",
                        "arguments": {
                            "question": "what is in front?"
                        }
                    },
                    "id": 2
                }
            })
        elif payload.get("id") == 2:
            log(f"Photo result: {payload}")
            await ws.close()
            break

    log("WebSocket connection closed")
    return ws


async def main() -> None:
    log("Backend starting")
    await http_server()


if __name__ == "__main__":
    asyncio.run(main())
