import { Button } from "@/components/ui/button";
import { Play, StepForward, RotateCcw } from "lucide-react";

interface CodeEditorProps {
  code: string;
  onChange: (code: string) => void;
  onExecute: () => void;
  onStep: () => void;
  onReset: () => void;
  isExecuting: boolean;
}

export default function CodeEditor({ 
  code, 
  onChange, 
  onExecute, 
  onStep, 
  onReset,
  isExecuting 
}: CodeEditorProps) {
  return (
    <div className="flex-1 flex flex-col min-w-0">
      <div className="bg-card border-b border-border px-4 py-2 flex items-center justify-between">
        <div className="flex items-center space-x-2">
          <span className="text-sm font-medium">main.vorax</span>
          <span className="text-xs text-muted-foreground">•</span>
          <span className="text-xs text-muted-foreground">VORAX-L</span>
        </div>
        <div className="flex items-center space-x-2">
          <Button 
            size="sm" 
            onClick={onExecute}
            disabled={isExecuting}
            data-testid="button-run"
          >
            <Play className="w-4 h-4 mr-1" />
            Run
          </Button>
          <Button 
            size="sm" 
            variant="secondary" 
            onClick={onStep}
            disabled={isExecuting}
            data-testid="button-step"
          >
            <StepForward className="w-4 h-4 mr-1" />
            Step
          </Button>
          <Button 
            size="sm" 
            variant="secondary" 
            onClick={onReset}
            data-testid="button-reset"
          >
            <RotateCcw className="w-4 h-4 mr-1" />
            Reset
          </Button>
        </div>
      </div>
      
      <div className="flex-1 p-4">
        <textarea 
          className="code-editor w-full h-full resize-none" 
          placeholder="Enter VORAX-L code here..."
          value={code}
          onChange={(e) => onChange(e.target.value)}
          data-testid="textarea-code-editor"
        />
      </div>
    </div>
  );
}
