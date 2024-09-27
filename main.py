from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from gpiozero import OutputDevice
import threading
import time

app = FastAPI()

ZONE_PINS = {
    1: OutputDevice(16),
    2: OutputDevice(20),
    3: OutputDevice(21),
}

class Duration(BaseModel):
    duration: float  # seconds

current_thread = None
stop_thread = False

def turn_on_zone_pin(zone_id: int, duration: float):
    global stop_thread

    # Turn off all zones
    for device in ZONE_PINS.values():
        device.off()

    # Turn on the selected zone
    pin = ZONE_PINS[zone_id]
    pin.on()

    start_time = time.time()
    while time.time() - start_time < duration:
        if stop_thread:
            break
        time.sleep(0.1)

    pin.off()

@app.post("/zones/{zone_id}")
def toggle(zone_id: int, duration: Duration):
    global current_thread, stop_thread

    if zone_id not in ZONE_PINS:
        raise HTTPException(status_code=400, detail="Invalid zone ID")

    # Stop the previous thread if it's still running
    if current_thread and current_thread.is_alive():
        stop_thread = True
        current_thread.join()  # Wait for the thread to stop

    # Reset stop signal and start a new thread for the new zone
    stop_thread = False
    current_thread = threading.Thread(target=turn_on_zone_pin, args=(zone_id, duration.duration))
    current_thread.start()

    return {"message": f"Zone {zone_id} activated for {duration.duration} seconds"}

# Endpoint to turn off all zones
@app.post("/zones")
def turn_off_all_zones():
    global stop_thread

    # Stop any running thread
    if current_thread and current_thread.is_alive():
        stop_thread = True
        current_thread.join()

    # Turn off all zones
    for device in ZONE_PINS.values():
        device.off()

    return {"message": "All zones turned off"}
