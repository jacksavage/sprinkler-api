from contextlib import asynccontextmanager
from fastapi import FastAPI, HTTPException
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel
from gpiozero.pins.native import NativeFactory
from gpiozero import OutputDevice
import threading
from typing import List

factory = NativeFactory()
app = FastAPI()

ZONE_PINS = {
    1: OutputDevice(16, pin_factory=factory),
    2: OutputDevice(20, pin_factory=factory),
    3: OutputDevice(21, pin_factory=factory),
}

class ScheduleItem(BaseModel):
    zone_id: int
    duration_seconds: float

current_timer = None
stop_event = threading.Event()

def turn_off_all_zones():
    for device in ZONE_PINS.values():
        device.off()

def turn_on_zone(zone: int):
    turn_off_all_zones()

    if zone not in ZONE_PINS:
        raise HTTPException(status_code=400, detail=f"Zone {zone} does not exist")
    ZONE_PINS[zone].on()

def process_schedule(schedule: List[ScheduleItem]):
    try:
        # process each item in the schedule
        for item in schedule:
            # check the stop event
            if stop_event.is_set():
                break

            # turn on the zone
            turn_on_zone(item.zone_id)

            # wait for the duration of the item or the stop event
            stop_event.wait(item.duration_seconds)
    finally:
        # always turn off all zones when we are done
        turn_off_all_zones()

@app.post("/api/schedule")
def set_schedule(schedule: List[ScheduleItem]):
    global current_timer, stop_event

    # stop the previous schedule if it's still running
    if current_timer and current_timer.is_alive():
        # ask thread to stop running
        stop_event.set()

        # wait for it to do so
        current_timer.join()

    # if a schedule is provided
    if schedule:
        # run it on a new thread
        stop_event.clear()
        current_timer = threading.Thread(target=process_schedule, args=(schedule,))
        current_timer.start()
        return {"message": "New schedule started"}
    else:
        # otherwise no action is needed
        return {"message": "All zones off"}

# turn off all zones when the server starts and stops
@asynccontextmanager
async def lifespan(app: FastAPI):
    turn_off_all_zones()
    yield
    turn_off_all_zones()

# serve the client
app.mount("/", StaticFiles(directory="./static", html=True))
