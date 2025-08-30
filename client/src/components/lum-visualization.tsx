import { Zone } from "@shared/schema";

interface LumVisualizationProps {
  zones: Zone[];
  selectedZone?: string;
  onZoneSelect?: (zoneId: string) => void;
}

export default function LumVisualization({ zones, selectedZone, onZoneSelect }: LumVisualizationProps) {
  return (
    <div className="space-y-4">
      <h3 className="text-lg font-semibold">LUM Distribution</h3>
      <div className="grid grid-cols-2 gap-4">
        {zones.map((zone) => (
          <div
            key={zone.id}
            className={`p-4 border rounded-lg cursor-pointer transition-colors ${
              selectedZone === zone.id
                ? 'border-primary bg-primary/10'
                : 'border-border hover:border-primary/50'
            }`}
            onClick={() => onZoneSelect?.(zone.id)}
            data-testid={`zone-visualization-${zone.name}`}
          >
            <div className="flex items-center justify-between mb-2">
              <h4 className="font-medium">{zone.name}</h4>
              <span className="text-sm text-muted-foreground">
                {zone.lumCount} LUMs
              </span>
            </div>
            <div className="flex flex-wrap gap-1">
              {Array.from({ length: zone.lumCount }).map((_, i) => (
                <div
                  key={i}
                  className="w-3 h-3 bg-chart-2 rounded-full animate-pulse"
                  data-testid={`lum-${zone.name}-${i}`}
                />
              ))}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}
