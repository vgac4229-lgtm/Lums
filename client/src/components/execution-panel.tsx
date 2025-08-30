import { Zone, ExecutionStep, LogEntry, ExecutionMetrics } from "@shared/schema";
import { Button } from "@/components/ui/button";
import { Progress } from "@/components/ui/progress";
import { Play, StepForward, Pause, RotateCcw } from "lucide-react";

interface ExecutionPanelProps {
  currentTick: number;
  totalTicks: number;
  isExecuting: boolean;
  logs: LogEntry[];
  executionSteps: ExecutionStep[];
  zones: Zone[];
  metrics: ExecutionMetrics;
  onExecute: () => void;
  onStep: () => void;
  onPause: () => void;
  onReset: () => void;
}

export default function ExecutionPanel({
  currentTick,
  totalTicks,
  isExecuting,
  logs,
  executionSteps,
  zones,
  metrics,
  onExecute,
  onStep,
  onPause,
  onReset
}: ExecutionPanelProps) {
  const totalLums = zones.reduce((sum, zone) => sum + zone.lumCount, 0);
  
  return (
    <div className="w-80 bg-card border-l border-border flex flex-col">
      {/* Execution Controls */}
      <div className="p-4 border-b border-border">
        <h2 className="font-semibold text-sm text-foreground mb-3">Execution Control</h2>
        <div className="grid grid-cols-2 gap-2">
          <Button 
            onClick={onExecute}
            disabled={isExecuting}
            data-testid="button-execute"
          >
            <Play className="w-4 h-4 mr-1.5" />
            Execute
          </Button>
          <Button 
            variant="secondary" 
            onClick={onStep}
            disabled={isExecuting}
            data-testid="button-step-execution"
          >
            <StepForward className="w-4 h-4 mr-1.5" />
            Step
          </Button>
          <Button 
            variant="secondary" 
            onClick={onPause}
            disabled={!isExecuting}
            data-testid="button-pause"
          >
            <Pause className="w-4 h-4 mr-1.5" />
            Pause
          </Button>
          <Button 
            variant="secondary" 
            onClick={onReset}
            data-testid="button-reset-execution"
          >
            <RotateCcw className="w-4 h-4 mr-1.5" />
            Reset
          </Button>
        </div>
        
        <div className="mt-3 space-y-2">
          <div className="flex items-center justify-between text-sm">
            <span className="text-muted-foreground">Current Tick:</span>
            <span className="font-mono" data-testid="text-current-tick">{currentTick} / {totalTicks}</span>
          </div>
          <div className="flex items-center justify-between text-sm">
            <span className="text-muted-foreground">Total LUMs:</span>
            <span className="font-mono text-chart-2" data-testid="text-total-lums">{totalLums}</span>
          </div>
          <div className="flex items-center justify-between text-sm">
            <span className="text-muted-foreground">Conservation:</span>
            <span className="font-mono text-chart-2" data-testid="text-conservation">
              {metrics.conservationValid ? '✓ Valid' : '✗ Invalid'}
            </span>
          </div>
        </div>
      </div>

      {/* Execution Steps */}
      <div className="p-4 border-b border-border">
        <h2 className="font-semibold text-sm text-foreground mb-3">Execution Steps</h2>
        <div className="space-y-1 max-h-40 overflow-y-auto">
          {executionSteps.map((step, index) => (
            <div 
              key={index} 
              className={`execution-step ${index === currentTick - 1 ? 'active' : ''}`}
              data-testid={`execution-step-${index}`}
            >
              <div className="text-xs text-muted-foreground">Tick {step.tick}</div>
              <div className="text-sm">{step.operation}</div>
            </div>
          ))}
        </div>
      </div>

      {/* State Visualization */}
      <div className="p-4 border-b border-border">
        <h2 className="font-semibold text-sm text-foreground mb-3">State Visualization</h2>
        <div className="space-y-3">
          {zones.map((zone) => {
            const percentage = totalLums > 0 ? (zone.lumCount / totalLums) * 100 : 0;
            const chartColor = [
              'var(--chart-1)',
              'var(--chart-2)', 
              'var(--chart-3)',
              'var(--chart-4)'
            ][zones.indexOf(zone) % 4];
            
            return (
              <div key={zone.id} className="zone-container">
                <div className="flex items-center justify-between mb-2">
                  <span className="text-sm font-medium">{zone.name}</span>
                  <span 
                    className="text-xs px-2 py-1 rounded"
                    style={{ backgroundColor: chartColor }}
                    data-testid={`zone-count-${zone.name}`}
                  >
                    {zone.lumCount} LUMs
                  </span>
                </div>
                <Progress value={percentage} className="h-2" />
              </div>
            );
          })}
        </div>
      </div>

      {/* Log Output */}
      <div className="flex-1 flex flex-col min-h-0">
        <div className="p-4 pb-2">
          <h2 className="font-semibold text-sm text-foreground">Execution Log</h2>
        </div>
        <div className="flex-1 overflow-y-auto" data-testid="log-container">
          {logs.map((log, index) => (
            <div 
              key={index} 
              className={`log-entry ${log.level}`}
              data-testid={`log-entry-${index}`}
            >
              [{log.timestamp}] {log.message}
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
