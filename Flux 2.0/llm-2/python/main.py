import os
import socket
import struct
import time
from google import genai
from google.genai import types

# 1. Route traffic through your working hardware bridge
def get_docker_gateway():
    try:
        with open("/proc/net/route") as f:
            for line in f:
                fields = line.strip().split()
                if fields[1] == '00000000':
                    return socket.inet_ntoa(struct.pack("<L", int(fields[2], 16)))
    except Exception:
        pass
    return "172.18.0.1"

HOST_ADDR = get_docker_gateway()
PROXY_PORT = 18080

os.environ["http_proxy"] = f"http://{HOST_ADDR}:{PROXY_PORT}"
os.environ["https_proxy"] = f"http://{HOST_ADDR}:{PROXY_PORT}"

# 2. Setup Gemini
API_KEY = "" 
client = genai.Client(api_key=API_KEY)

# 3. Load the compressed image
# Note: Since this is running from the python/ directory, it looks in the same folder
image_path = os.path.join(os.path.dirname(__file__), "test.jpg")

if not os.path.exists(image_path):
    print(f"ERROR: Cannot find {image_path}. Please upload a tiny JPEG (< 50KB) to the python folder.", flush=True)
    exit(1)

file_size = os.path.getsize(image_path)
print(f"Image size: {file_size / 1024:.1f} KB", flush=True)

with open(image_path, "rb") as f:
    image_bytes = f.read()

image_part = types.Part.from_bytes(data=image_bytes, mime_type="image/jpeg")

# 4. Send to Gemini
print("Sending image across the serial bridge to Gemini... (This will take a few seconds)", flush=True)
start_time = time.time()

try:
    response = client.models.generate_content(
        model="gemini-3.5-flash-lite", 
        contents=[image_part, "Describe exactly what object is in this image. in exactly 100 words"],
    )
    
    elapsed = time.time() - start_time
    print(f"\nSUCCESS! (Took {elapsed:.1f} seconds)", flush=True)
    print("Gemini says:", flush=True)
    print("-" * 40, flush=True)
    print(response.text, flush=True)
    print("-" * 40, flush=True)

except Exception as e:
    print(f"\nFAILED: {e}", flush=True)