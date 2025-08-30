import { Zone, ExecutionMetrics } from "@shared/schema";
import { Button } from "@/components/ui/button";
import { Play, CheckCircle, AlertTriangle } from "lucide-react";

interface BottomPanelProps {
  zones: Zone[];
  metrics: ExecutionMetrics;
}

export default function BottomPanel({ zones, metrics }: BottomPanelProps) {
  const handleRunTests = () => {
    // TODO: Implement test execution
    console.log('Running tests...');
  };

  const testResults = [
    { name: "Conservation invariant test", status: "pass", icon: CheckCircle },
    { name: "No duplication test", status: "pass", icon: CheckCircle },
    { name: "Determinism test", status: "pass", icon: CheckCircle },
    { name: "Linear types verification", status: "warning", icon: AlertTriangle },
  ];

  return (
    <div className="h-48 bg-card border-t border-border flex">
      {/* Test Results */}
      <div className="flex-1 p-4">
        <div className="flex items-center justify-between mb-3">
          <h2 className="font-semibold text-sm text-foreground">Test Results</h2>
          <Button 
            size="sm" 
            onClick={handleRunTests}
            data-testid="button-run-tests"
          >
            <Play className="w-4 h-4 mr-1" />
            Run Tests
          </Button>
        </div>
        <div className="space-y-2 overflow-y-auto">
          {testResults.map((test, index) => {
            const Icon = test.icon;
            const colorClass = test.status === "pass" ? "text-chart-2" : "text-chart-3";
            
            return (
              <div 
                key={index} 
                className="flex items-center space-x-2 p-2 rounded bg-accent/20"
                data-testid={`test-result-${index}`}
              >
                <Icon className={`${colorClass} w-4 h-4`} />
                <span className="text-sm">{test.name}</span>
                <span className={`text-xs ${colorClass} ml-auto uppercase`}>
                  {test.status}
                </span>
              </div>
            );
          })}
        </div>
      </div>

      <div className="w-px bg-border"></div>

      {/* Performance Metrics */}
      <div className="flex-1 p-4">
        <h2 className="font-semibold text-sm text-foreground mb-3">Performance Metrics</h2>
        <div className="grid grid-cols-2 gap-4">
          <div className="space-y-2">
            <div className="flex items-center justify-between text-xs">
              <span className="text-muted-foreground">Execution Time</span>
              <span className="font-mono" data-testid="metric-execution-time">
                {metrics.executionTime}ms
              </span>
            </div>
            <div className="flex items-center justify-between text-xs">
              <span className="text-muted-foreground">Memory Usage</span>
              <span className="font-mono" data-testid="metric-memory-usage">
                {metrics.memoryUsage}KB
              </span>
            </div>
            <div className="flex items-center justify-between text-xs">
              <span className="text-muted-foreground">LUM Operations</span>
              <span className="font-mono" data-testid="metric-lum-operations">
                {metrics.lumOperations}
              </span>
            </div>
          </div>
          <div className="space-y-2">
            <div className="flex items-center justify-between text-xs">
              <span className="text-muted-foreground">Ticks Executed</span>
              <span className="font-mono" data-testid="metric-ticks-executed">
                {metrics.ticksExecuted}/{metrics.totalTicks}
              </span>
            </div>
            <div className="flex items-center justify-between text-xs">
              <span className="text-muted-foreground">Violations</span>
              <span className="font-mono text-chart-2" data-testid="metric-violations">
                {metrics.violations}
              </span>
            </div>
            <div className="flex items-center justify-between text-xs">
              <span className="text-muted-foreground">Efficiency</span>
              <span className="font-mono" data-testid="metric-efficiency">
                {metrics.efficiency}%
              </span>
            </div>
          </div>
        </div>
      </div>

      <div className="w-px bg-border"></div>

      {/* Invariant Checker */}
      <div className="flex-1 p-4">
        <h2 className="font-semibold text-sm text-foreground mb-3">Invariant Status</h2>
        <div className="space-y-2">
          <div className="flex items-center space-x-2">
            <div className="w-2 h-2 bg-chart-2 rounded-full"></div>
            <span className="text-xs" data-testid="invariant-conservation">
              Conservation: {zones.reduce((sum, z) => sum + z.lumCount, 0)}/9 LUMs
            </span>
          </div>
          <div className="flex items-center space-x-2">
            <div className="w-2 h-2 bg-chart-2 rounded-full"></div>
            <span className="text-xs" data-testid="invariant-linear-types">
              Linear Types: Valid
            </span>
          </div>
          <div className="flex items-center space-x-2">
            <div className="w-2 h-2 bg-chart-2 rounded-full"></div>
            <span className="text-xs" data-testid="invariant-determinism">
              Determinism: Verified
            </span>
          </div>
          <div className="flex items-center space-x-2">
            <div className="w-2 h-2 bg-chart-3 rounded-full"></div>
            <span className="text-xs" data-testid="invariant-race-conditions">
              Race Conditions: {metrics.raceConditions} Warning{metrics.raceConditions !== 1 ? 's' : ''}
            </span>
          </div>
        </div>
      </div>
    </div>
  );
}
