import asyncio
import json
import uuid

from aiohttp import web
import websockets


HOST_IP = "192.168.1.18"
HTTP_PORT = 8000
WS_PORT = 8765
SESSION_ID = str(uuid.uuid4())
CAPTURE_PATH = "capture.jpg"


async def ota_handler(request: web.Request) -> web.StreamResponse:
    return web.json_response({
        "firmware": {"version": "local"},
        "websocket": {
            "url": f"ws://{HOST_IP}:{WS_PORT}/ws",
            "version": 3,
            "token": ""
        }
    })


async def upload_handler(request: web.Request) -> web.StreamResponse:
    reader = await request.multipart()
    question = None
    image_bytes = bytearray()

    async for part in reader:
        if part.name == "question":
            question = await part.text()
        elif part.name == "file":
            while True:
                chunk = await part.read_chunk()
                if not chunk:
                    break
                image_bytes.extend(chunk)

    with open(CAPTURE_PATH, "wb") as file:
        file.write(image_bytes)

    return web.json_response({"success": True, "question": question or ""})


async def http_server() -> None:
    app = web.Application()
    app.router.add_route("GET", "/ota.json", ota_handler)
    app.router.add_route("POST", "/ota.json", ota_handler)
    app.router.add_post("/upload", upload_handler)

    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, "0.0.0.0", HTTP_PORT)
    await site.start()


async def ws_handler(websocket: websockets.WebSocketServerProtocol) -> None:
    hello = json.loads(await websocket.recv())
    print(">> hello:", hello)

    await websocket.send(json.dumps({
        "type": "hello",
        "transport": "websocket",
        "session_id": SESSION_ID,
        "audio_params": {
            "format": "opus",
            "sample_rate": 16000
        }
    }))

    await asyncio.sleep(0.5)
    await websocket.send(json.dumps({
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
    }))

    async for raw in websocket:
        message = json.loads(raw)
        print(">>", message)

        if message.get("type") != "mcp":
            continue

        payload = message.get("payload", {})
        if payload.get("id") == 1 and "result" in payload:
            await websocket.send(json.dumps({
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
            }))
        elif payload.get("id") == 2:
            print("photo result:", payload)
            await websocket.close()
            break


async def ws_server() -> None:
    async with websockets.serve(ws_handler, "0.0.0.0", WS_PORT, ping_interval=None):
        await asyncio.Future()


async def main() -> None:
    await asyncio.gather(http_server(), ws_server())


if __name__ == "__main__":
    asyncio.run(main())
