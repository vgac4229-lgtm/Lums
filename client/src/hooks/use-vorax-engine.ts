import { useState, useCallback } from 'react';
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';
import { Zone, MemorySlot, LogEntry, ExecutionStep, ExecutionMetrics } from '@shared/schema';
import { VoraxParser } from '@/lib/vorax-parser';

export function useVoraxEngine() {
  const queryClient = useQueryClient();
  const [currentTick, setCurrentTick] = useState(0);
  const [totalTicks, setTotalTicks] = useState(0);
  const [isExecuting, setIsExecuting] = useState(false);

  // Fetch current engine state
  const { data: engineState } = useQuery({
    queryKey: ['/api/vorax/state'],
    queryFn: async () => {
      const response = await fetch('/api/vorax/state');
      if (!response.ok) throw new Error('Failed to fetch engine state');
      return response.json();
    },
    refetchInterval: isExecuting ? 1000 : false,
  });

  const zones: Zone[] = engineState?.zones || [
    { id: 'A', name: 'Zone A', lumCount: 0, bounds: { x: 0, y: 0, width: 100, height: 100 } },
    { id: 'B', name: 'Zone B', lumCount: 0, bounds: { x: 0, y: 0, width: 100, height: 100 } },
    { id: 'C', name: 'Zone C', lumCount: 0, bounds: { x: 0, y: 0, width: 100, height: 100 } },
    { id: 'D', name: 'Zone D', lumCount: 0, bounds: { x: 0, y: 0, width: 100, height: 100 } },
  ];

  const memory: MemorySlot[] = engineState?.memory || [
    { id: 'buf', name: 'buf', lumCount: 0, timestamp: Date.now() },
    { id: 'cache', name: 'cache', lumCount: 0, timestamp: Date.now() },
  ];

  const logs: LogEntry[] = engineState?.logs || [];
  const executionSteps: ExecutionStep[] = engineState?.executionSteps || [];
  const metrics: ExecutionMetrics = engineState?.metrics || {
    executionTime: 0,
    memoryUsage: 0,
    lumOperations: 0,
    ticksExecuted: 0,
    totalTicks: 0,
    violations: 0,
    efficiency: 100,
    conservationValid: true,
    raceConditions: 0,
  };

  // Execute code mutation
  const executeCodeMutation = useMutation({
    mutationFn: async (code: string) => {
      const parser = new VoraxParser();
      const statements = parser.parse(code);
      
      const response = await fetch('/api/vorax/execute', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ code, statements }),
      });
      
      if (!response.ok) {
        const error = await response.json();
        throw new Error(error.message || 'Execution failed');
      }
      
      return response.json();
    },
    onSuccess: (data) => {
      setCurrentTick(data.currentTick || 0);
      setTotalTicks(data.totalTicks || 0);
      setIsExecuting(data.isExecuting || false);
      queryClient.invalidateQueries({ queryKey: ['/api/vorax/state'] });
    },
  });

  // Step execution mutation
  const stepExecutionMutation = useMutation({
    mutationFn: async () => {
      const response = await fetch('/api/vorax/step', {
        method: 'POST',
      });
      
      if (!response.ok) {
        const error = await response.json();
        throw new Error(error.message || 'Step execution failed');
      }
      
      return response.json();
    },
    onSuccess: (data) => {
      setCurrentTick(data.currentTick || 0);
      queryClient.invalidateQueries({ queryKey: ['/api/vorax/state'] });
    },
  });

  // Pause execution mutation
  const pauseExecutionMutation = useMutation({
    mutationFn: async () => {
      const response = await fetch('/api/vorax/pause', {
        method: 'POST',
      });
      
      if (!response.ok) {
        const error = await response.json();
        throw new Error(error.message || 'Pause execution failed');
      }
      
      return response.json();
    },
    onSuccess: () => {
      setIsExecuting(false);
      queryClient.invalidateQueries({ queryKey: ['/api/vorax/state'] });
    },
  });

  // Reset execution mutation
  const resetExecutionMutation = useMutation({
    mutationFn: async () => {
      const response = await fetch('/api/vorax/reset', {
        method: 'POST',
      });
      
      if (!response.ok) {
        const error = await response.json();
        throw new Error(error.message || 'Reset execution failed');
      }
      
      return response.json();
    },
    onSuccess: () => {
      setCurrentTick(0);
      setTotalTicks(0);
      setIsExecuting(false);
      queryClient.invalidateQueries({ queryKey: ['/api/vorax/state'] });
    },
  });

  const executeCode = useCallback((code: string) => {
    return executeCodeMutation.mutateAsync(code);
  }, [executeCodeMutation]);

  const stepExecution = useCallback(() => {
    return stepExecutionMutation.mutateAsync();
  }, [stepExecutionMutation]);

  const pauseExecution = useCallback(() => {
    return pauseExecutionMutation.mutateAsync();
  }, [pauseExecutionMutation]);

  const resetExecution = useCallback(() => {
    return resetExecutionMutation.mutateAsync();
  }, [resetExecutionMutation]);

  return {
    zones,
    memory,
    currentTick,
    totalTicks,
    isExecuting,
    logs,
    executionSteps,
    metrics,
    executeCode,
    stepExecution,
    pauseExecution,
    resetExecution,
  };
}
