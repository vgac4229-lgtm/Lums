
import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Progress } from '@/components/ui/progress';
import CodeEditor from '@/components/code-editor';
import ExecutionPanel from '@/components/execution-panel';
import { useVoraxEngine } from '@/hooks/use-vorax-engine';

export default function IDEPage() {
  const [voraxCode, setVoraxCode] = useState(`# VORAX-L Example Program
zone A, B, C, D;
mem buf, cache;

# Initialize zones
emit A += 3•;
emit B += 2•;

# Main program loop
fuse A B -> C;
split C -> [D, D, D];
cycle D % 2;
store buf <- D, 1•;
retrieve buf -> A;

# Handlers
on (empty A) {
  emit A += 1•;
}

every 5 {
  compress D -> Ω, cost=3;
  expand Ω -> [A, B] factor=2;
}`);

  const [compilationResult, setCompilationResult] = useState<any>(null);
  const [executionResult, setExecutionResult] = useState<any>(null);
  const [isCompiling, setIsCompiling] = useState(false);
  const [isExecuting, setIsExecuting] = useState(false);
  const [bootstrapStatus, setBootstrapStatus] = useState<any>({});

  const { engine, compileAndExecute, getEngineState } = useVoraxEngine();

  const handleCompile = async () => {
    setIsCompiling(true);
    try {
      const response = await fetch('/api/vorax/compile', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ code: voraxCode })
      });
      
      const result = await response.json();
      setCompilationResult(result);
    } catch (error) {
      setCompilationResult({ 
        success: false, 
        error: error instanceof Error ? error.message : 'Compilation failed' 
      });
    } finally {
      setIsCompiling(false);
    }
  };

  const handleExecute = async () => {
    if (!compilationResult?.success) {
      alert('Please compile the code first');
      return;
    }

    setIsExecuting(true);
    try {
      const response = await fetch('/api/vorax/execute', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ 
          instructions: compilationResult.instructions,
          maxTicks: 100
        })
      });
      
      const result = await response.json();
      setExecutionResult(result);
    } catch (error) {
      setExecutionResult({ 
        success: false, 
        error: error instanceof Error ? error.message : 'Execution failed' 
      });
    } finally {
      setIsExecuting(false);
    }
  };

  const handleBootstrap = async () => {
    const phases = [
      'vorax_bootstrap_phase1',
      'vorax_bootstrap_phase2',
      'vorax_bootstrap_phase3',
      'vorax_bootstrap_phase4',
      'vorax_bootstrap_phase5',
      'vorax_bootstrap_phase6'
    ];

    for (const phase of phases) {
      setBootstrapStatus(prev => ({ ...prev, [phase]: 'running' }));
      
      try {
        const response = await fetch(`/api/bootstrap/${phase}`, {
          method: 'POST'
        });
        
        const result = await response.json();
        setBootstrapStatus(prev => ({ 
          ...prev, 
          [phase]: result.success ? 'completed' : 'failed' 
        }));
      } catch (error) {
        setBootstrapStatus(prev => ({ ...prev, [phase]: 'failed' }));
      }
    }
  };

  const getBootstrapProgress = () => {
    const phases = Object.keys(bootstrapStatus);
    const completed = phases.filter(phase => bootstrapStatus[phase] === 'completed').length;
    return phases.length > 0 ? (completed / phases.length) * 100 : 0;
  };

  return (
    <div className="container mx-auto p-6 space-y-6">
      <div className="flex justify-between items-center">
        <h1 className="text-3xl font-bold">VORAX-L IDE</h1>
        <div className="space-x-2">
          <Button onClick={handleBootstrap} variant="outline">
            Bootstrap System
          </Button>
          <Button onClick={handleCompile} disabled={isCompiling}>
            {isCompiling ? 'Compiling...' : 'Compile'}
          </Button>
          <Button onClick={handleExecute} disabled={isExecuting || !compilationResult?.success}>
            {isExecuting ? 'Executing...' : 'Execute'}
          </Button>
        </div>
      </div>

      {Object.keys(bootstrapStatus).length > 0 && (
        <Card>
          <CardHeader>
            <CardTitle>Bootstrap Progress</CardTitle>
          </CardHeader>
          <CardContent>
            <Progress value={getBootstrapProgress()} className="mb-4" />
            <div className="grid grid-cols-2 gap-2 text-sm">
              {Object.entries(bootstrapStatus).map(([phase, status]) => (
                <div key={phase} className="flex justify-between">
                  <span>{phase.replace('vorax_bootstrap_', 'Phase ')}</span>
                  <span className={
                    status === 'completed' ? 'text-green-600' :
                    status === 'running' ? 'text-yellow-600' :
                    status === 'failed' ? 'text-red-600' : ''
                  }>
                    {status}
                  </span>
                </div>
              ))}
            </div>
          </CardContent>
        </Card>
      )}

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        <Card>
          <CardHeader>
            <CardTitle>VORAX-L Code Editor</CardTitle>
          </CardHeader>
          <CardContent>
            <CodeEditor
              value={voraxCode}
              onChange={setVoraxCode}
              language="vorax"
              height="500px"
            />
          </CardContent>
        </Card>

        <Card>
          <CardHeader>
            <CardTitle>Execution Visualization</CardTitle>
          </CardHeader>
          <CardContent>
            <ExecutionPanel 
              executionResult={executionResult}
              engineState={getEngineState()}
            />
          </CardContent>
        </Card>
      </div>

      <Tabs defaultValue="compilation" className="w-full">
        <TabsList>
          <TabsTrigger value="compilation">Compilation</TabsTrigger>
          <TabsTrigger value="execution">Execution</TabsTrigger>
          <TabsTrigger value="logs">Logs</TabsTrigger>
          <TabsTrigger value="c-output">C Output</TabsTrigger>
        </TabsList>

        <TabsContent value="compilation">
          <Card>
            <CardHeader>
              <CardTitle>Compilation Results</CardTitle>
            </CardHeader>
            <CardContent>
              {compilationResult ? (
                <div className="space-y-4">
                  {compilationResult.success ? (
                    <Alert>
                      <AlertDescription>
                        Compilation successful! Generated {compilationResult.instructions?.length || 0} V-IR instructions.
                      </AlertDescription>
                    </Alert>
                  ) : (
                    <Alert variant="destructive">
                      <AlertDescription>
                        Compilation failed: {compilationResult.error}
                      </AlertDescription>
                    </Alert>
                  )}
                  
                  {compilationResult.instructions && (
                    <div>
                      <h4 className="font-semibold mb-2">V-IR Instructions:</h4>
                      <pre className="bg-gray-100 p-4 rounded text-sm overflow-auto">
                        {JSON.stringify(compilationResult.instructions, null, 2)}
                      </pre>
                    </div>
                  )}
                </div>
              ) : (
                <p className="text-gray-500">Click 'Compile' to see results</p>
              )}
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="execution">
          <Card>
            <CardHeader>
              <CardTitle>Execution Results</CardTitle>
            </CardHeader>
            <CardContent>
              {executionResult ? (
                <div className="space-y-4">
                  {executionResult.success ? (
                    <div>
                      <Alert>
                        <AlertDescription>
                          Execution completed in {executionResult.ticksExecuted} ticks, 
                          using {executionResult.energyUsed} energy units.
                        </AlertDescription>
                      </Alert>
                      
                      <div className="mt-4">
                        <h4 className="font-semibold mb-2">Final State:</h4>
                        <div className="grid grid-cols-4 gap-2 text-sm">
                          {executionResult.finalState?.zones.map((count, index) => (
                            <div key={index} className="bg-gray-100 p-2 rounded text-center">
                              <div className="font-semibold">Zone {String.fromCharCode(65 + index)}</div>
                              <div>{count} LUMs</div>
                            </div>
                          ))}
                        </div>
                      </div>
                    </div>
                  ) : (
                    <Alert variant="destructive">
                      <AlertDescription>
                        Execution failed: {executionResult.error}
                      </AlertDescription>
                    </Alert>
                  )}
                </div>
              ) : (
                <p className="text-gray-500">Click 'Execute' to see results</p>
              )}
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="logs">
          <Card>
            <CardHeader>
              <CardTitle>System Logs</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-64 overflow-auto bg-black text-green-400 p-4 rounded font-mono text-sm">
                {executionResult?.executionLog?.map((log, index) => (
                  <div key={index}>
                    Tick {log.tick}: Opcode 0x{log.opcode.toString(16)} - Energy: {log.energyCost}
                  </div>
                )) || 'No logs available'}
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="c-output">
          <Card>
            <CardHeader>
              <CardTitle>Generated C Code</CardTitle>
            </CardHeader>
            <CardContent>
              {compilationResult?.cCode ? (
                <pre className="bg-gray-100 p-4 rounded text-sm overflow-auto h-64">
                  {compilationResult.cCode}
                </pre>
              ) : (
                <p className="text-gray-500">Compile code to see generated C output</p>
              )}
            </CardContent>
          </Card>
        </TabsContent>
      </Tabs>
    </div>
  );
}
