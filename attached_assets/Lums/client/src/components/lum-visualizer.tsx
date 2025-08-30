import { useState, useRef, useEffect } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import type { LUM, LUMGroup } from "@shared/schema";

export default function LumVisualizer() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [zones, setZones] = useState<Record<string, LUMGroup>>({
    'Zone A': {
      id: 'zoneA',
      lums: [
        { presence: 1, structureType: 'linear', position: { x: 50, y: 50 } },
        { presence: 1, structureType: 'linear', position: { x: 70, y: 50 } },
        { presence: 1, structureType: 'linear', position: { x: 90, y: 50 } },
      ],
      groupType: 'linear',
    },
    'Zone B': {
      id: 'zoneB',
      lums: [
        { presence: 1, structureType: 'linear', position: { x: 300, y: 50 } },
        { presence: 1, structureType: 'linear', position: { x: 320, y: 50 } },
      ],
      groupType: 'linear',
    },
  });

  const [animationState, setAnimationState] = useState<'idle' | 'fusion' | 'split'>('idle');

  useEffect(() => {
    drawVisualization();
  }, [zones, animationState]);

  const drawVisualization = () => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Set canvas style
    ctx.fillStyle = 'hsl(158, 64%, 52%)'; // Secondary color
    ctx.strokeStyle = 'hsl(43, 96%, 56%)'; // Accent color

    // Draw zones
    Object.entries(zones).forEach(([zoneName, zone], zoneIndex) => {
      // Draw zone boundary
      const zoneX = zoneIndex * 200 + 20;
      const zoneY = 20;
      const zoneWidth = 160;
      const zoneHeight = 100;

      ctx.strokeRect(zoneX, zoneY, zoneWidth, zoneHeight);
      
      // Draw zone label
      ctx.fillStyle = 'hsl(240, 4%, 46%)'; // Muted foreground
      ctx.font = '12px Inter';
      ctx.fillText(zoneName, zoneX + 5, zoneY + 15);

      // Draw LUMs in zone
      ctx.fillStyle = 'hsl(158, 64%, 52%)'; // Secondary color
      zone.lums.forEach((lum, lumIndex) => {
        if (lum.presence === 1) {
          const x = zoneX + 20 + (lumIndex * 20);
          const y = zoneY + 40;
          
          // Draw LUM dot with glow effect
          ctx.beginPath();
          ctx.arc(x, y, 6, 0, 2 * Math.PI);
          ctx.fill();
          
          // Add glow effect
          ctx.beginPath();
          ctx.arc(x, y, 12, 0, 2 * Math.PI);
          ctx.strokeStyle = 'hsl(158, 64%, 52%, 0.3)';
          ctx.lineWidth = 2;
          ctx.stroke();
          ctx.lineWidth = 1;
        }
      });
    });

    // Draw connections during fusion animation
    if (animationState === 'fusion') {
      ctx.strokeStyle = 'hsl(43, 96%, 56%)'; // Accent color
      ctx.setLineDash([5, 5]);
      ctx.beginPath();
      ctx.moveTo(120, 70);
      ctx.quadraticCurveTo(200, 30, 300, 70);
      ctx.stroke();
      ctx.setLineDash([]);
    }
  };

  const executeFusion = () => {
    setAnimationState('fusion');
    
    // Animate fusion after a delay
    setTimeout(() => {
      const zoneALums = zones['Zone A'].lums;
      const zoneBLums = zones['Zone B'].lums;
      const fusedLums = [...zoneALums, ...zoneBLums];
      
      setZones({
        ...zones,
        'Zone A': {
          ...zones['Zone A'],
          lums: fusedLums,
          groupType: 'cluster',
        },
        'Zone B': {
          ...zones['Zone B'],
          lums: [],
        },
      });
      
      setAnimationState('idle');
    }, 1000);
  };

  const executeSplit = () => {
    setAnimationState('split');
    
    setTimeout(() => {
      const allLums = zones['Zone A'].lums;
      const mid = Math.ceil(allLums.length / 2);
      
      setZones({
        'Zone A': {
          ...zones['Zone A'],
          lums: allLums.slice(0, mid),
        },
        'Zone B': {
          ...zones['Zone B'],
          lums: allLums.slice(mid),
        },
      });
      
      setAnimationState('idle');
    }, 1000);
  };

  const resetZones = () => {
    setZones({
      'Zone A': {
        id: 'zoneA',
        lums: [
          { presence: 1, structureType: 'linear', position: { x: 50, y: 50 } },
          { presence: 1, structureType: 'linear', position: { x: 70, y: 50 } },
          { presence: 1, structureType: 'linear', position: { x: 90, y: 50 } },
        ],
        groupType: 'linear',
      },
      'Zone B': {
        id: 'zoneB',
        lums: [
          { presence: 1, structureType: 'linear', position: { x: 300, y: 50 } },
          { presence: 1, structureType: 'linear', position: { x: 320, y: 50 } },
        ],
        groupType: 'linear',
      },
    });
    setAnimationState('idle');
  };

  return (
    <Card>
      <CardHeader>
        <CardTitle>Espace de Transformation</CardTitle>
      </CardHeader>
      <CardContent>
        <div className="grid lg:grid-cols-3 gap-8">
          <div className="lg:col-span-2">
            <div className="relative">
              <canvas
                ref={canvasRef}
                width={400}
                height={150}
                className="border border-border rounded-lg bg-card w-full"
                data-testid="visualization-canvas"
              />
              
              {/* Zone overlays for better UX */}
              <div className="absolute top-4 left-4 text-xs text-muted-foreground pointer-events-none">
                Zone A
              </div>
              <div className="absolute top-4 right-4 text-xs text-muted-foreground pointer-events-none">
                Zone B
              </div>
            </div>
          </div>

          <div>
            <h3 className="text-lg font-semibold mb-4">Contrôles</h3>
            <div className="space-y-3">
              <Button
                className="w-full bg-primary/20 border border-primary text-primary hover:bg-primary/30"
                onClick={executeFusion}
                disabled={animationState !== 'idle'}
                data-testid="button-fusion"
              >
                ⧉ Fusionner Zones
              </Button>
              
              <Button
                className="w-full bg-secondary/20 border border-secondary text-secondary hover:bg-secondary/30"
                onClick={executeSplit}
                disabled={animationState !== 'idle'}
                data-testid="button-split"
              >
                ⇅ Répartir
              </Button>
              
              <Button
                className="w-full bg-muted border border-border text-foreground hover:bg-muted/80"
                onClick={resetZones}
                disabled={animationState !== 'idle'}
                data-testid="button-reset"
              >
                ↻ Réinitialiser
              </Button>
            </div>

            <div className="mt-6 pt-6 border-t border-border">
              <h4 className="font-medium mb-3">État des Zones</h4>
              <div className="space-y-2 text-sm">
                {Object.entries(zones).map(([zoneName, zone]) => (
                  <div key={zoneName} className="vorax-syntax p-2 rounded text-xs">
                    {zoneName}: {zone.lums.length} LUM{zone.lums.length !== 1 ? 'S' : ''}
                  </div>
                ))}
              </div>
            </div>
          </div>
        </div>
      </CardContent>
    </Card>
  );
}
