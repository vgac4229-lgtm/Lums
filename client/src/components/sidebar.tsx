import { Zone, MemorySlot } from "@shared/schema";
import { FileCode, Settings, Cog } from "lucide-react";

interface SidebarProps {
  zones: Zone[];
  memory: MemorySlot[];
}

export default function Sidebar({ zones, memory }: SidebarProps) {
  return (
    <aside className="w-64 bg-card border-r border-border flex flex-col">
      <div className="p-4 border-b border-border">
        <h2 className="font-semibold text-sm text-foreground mb-3">Project Files</h2>
        <div className="space-y-1">
          <div className="flex items-center space-x-2 p-2 rounded-md bg-accent text-accent-foreground">
            <FileCode className="w-4 h-4" />
            <span className="text-sm">main.vorax</span>
          </div>
          <div className="flex items-center space-x-2 p-2 rounded-md hover:bg-accent/50 cursor-pointer">
            <FileCode className="w-4 h-4 text-muted-foreground" />
            <span className="text-sm text-muted-foreground">tests.vorax</span>
          </div>
          <div className="flex items-center space-x-2 p-2 rounded-md hover:bg-accent/50 cursor-pointer">
            <Settings className="w-4 h-4 text-muted-foreground" />
            <span className="text-sm text-muted-foreground">config.json</span>
          </div>
        </div>
      </div>
      
      <div className="p-4 border-b border-border">
        <h2 className="font-semibold text-sm text-foreground mb-3">LUM Zones</h2>
        <div className="space-y-2">
          {zones.map((zone) => (
            <div key={zone.id} className="flex items-center justify-between p-2 rounded-md bg-accent/30">
              <span className="text-sm font-medium">{zone.name}</span>
              <div className="flex items-center space-x-1">
                {zone.lumCount > 0 ? (
                  <>
                    {Array.from({ length: Math.min(zone.lumCount, 5) }).map((_, i) => (
                      <span key={i} className="lum-dot"></span>
                    ))}
                    <span className="text-xs text-muted-foreground ml-2">
                      {zone.lumCount} LUM{zone.lumCount !== 1 ? 's' : ''}
                    </span>
                  </>
                ) : (
                  <span className="text-xs text-muted-foreground">Empty</span>
                )}
              </div>
            </div>
          ))}
        </div>
      </div>
      
      <div className="p-4 flex-1">
        <h2 className="font-semibold text-sm text-foreground mb-3">Memory Buffers</h2>
        <div className="space-y-2">
          {memory.map((slot) => (
            <div key={slot.id} className="flex items-center justify-between p-2 rounded-md bg-accent/20">
              <span className="text-sm font-medium">{slot.name}</span>
              <div className="flex items-center space-x-1">
                {slot.lumCount > 0 ? (
                  <>
                    {Array.from({ length: Math.min(slot.lumCount, 3) }).map((_, i) => (
                      <span key={i} className="lum-dot"></span>
                    ))}
                    <span className="text-xs text-muted-foreground ml-2">
                      {slot.lumCount} LUM{slot.lumCount !== 1 ? 's' : ''}
                    </span>
                  </>
                ) : (
                  <span className="text-xs text-muted-foreground">Empty</span>
                )}
              </div>
            </div>
          ))}
        </div>
      </div>
    </aside>
  );
}
