import { useState } from "react"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { Label } from "@/components/ui/label"
import { Card, CardContent, CardFooter, CardHeader, CardTitle } from "@/components/ui/card"
import { Switch } from "@/components/ui/switch"

interface Zone {
  id: number
  duration: number
  enabled: boolean
}

export function SprinklerControl() {
  const [zones, setZones] = useState<Zone[]>([
    { id: 1, duration: 10, enabled: true },
    { id: 2, duration: 10, enabled: true },
    { id: 3, duration: 10, enabled: true }
  ])

  const updateDuration = (id: number, duration: number) => {
    setZones(zones.map(zone => zone.id === id ? { ...zone, duration } : zone))
  }

  const toggleZone = (id: number) => {
    setZones(zones.map(zone => zone.id === id ? { ...zone, enabled: !zone.enabled } : zone))
  }

  const executeSchedule = async () => {
    try {
      const response = await fetch("/api/schedule", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(
          zones
            .filter((zone) => zone.enabled)
            .map((zone) => ({
              zone_id: zone.id,
              duration_seconds: zone.duration * 60,
            }))
        ),
      });
      if (response.ok) {
        alert("Schedule execution command sent successfully!");
      } else {
        alert("Failed to send execute schedule command");
      }
    } catch (error) {
      console.error("Error sending execute schedule command:", error);
      alert("An error occurred while sending the execute schedule command");
    }
  };

  const stopSprinklers = async () => {
    try {
      const response = await fetch("/api/schedule", {
        method: "POST",
      });
      if (response.ok) {
        alert("Stop command sent successfully!");
      } else {
        alert("Failed to send stop command");
      }
    } catch (error) {
      console.error("Error sending stop command:", error);
      alert("An error occurred while sending the stop command");
    }
  };

  return (
    <Card className="w-full max-w-md mx-auto">
      <CardHeader>
        <CardTitle>Sprinkler Control</CardTitle>
      </CardHeader>
      <CardContent className="space-y-4">
        {zones.map(zone => (
          <div key={zone.id} className="flex items-center space-x-2">
            <Label htmlFor={`zone-${zone.id}`} className="w-20">Zone {zone.id}</Label>
            <Input
              id={`zone-${zone.id}`}
              type="number"
              value={zone.duration}
              onChange={(e) => updateDuration(zone.id, parseInt(e.target.value))}
              min="1"
              className="w-24"
              disabled={!zone.enabled}
            />
            <Label htmlFor={`zone-${zone.id}`} className="ml-2 w-16">minutes</Label>
            <Switch
              checked={zone.enabled}
              onCheckedChange={() => toggleZone(zone.id)}
              aria-label={`Toggle Zone ${zone.id}`}
            />
          </div>
        ))}
      </CardContent>
      <CardFooter className="flex flex-col space-y-2">
        <div className="flex space-x-2 w-full">
          <Button onClick={executeSchedule} className="flex-1">
            Execute Schedule
          </Button>
          <Button onClick={stopSprinklers} className="flex-1" variant="destructive">
            Stop
          </Button>
        </div>
      </CardFooter>
    </Card>
  )
}