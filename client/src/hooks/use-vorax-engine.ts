import { useState, useEffect, useCallback } from 'react';
import { VoraxEngineState, VoraxOperation } from '@shared/schema';

export function useVoraxEngine() {
  const [engineState, setEngineState] = useState<VoraxEngineState>({
    zones: {},
    memory: {},
    currentTick: 0,
    energyBudget: 1000,
    conservationValid: true
  });

  const [isConnected, setIsConnected] = useState(false);

  useEffect(() => {
    // Initialize connection to VORAX engine
    checkConnection();
  }, []);

  const checkConnection = async () => {
    try {
      const response = await fetch('/api/vorax/status');
      setIsConnected(response.ok);

      if (response.ok) {
        const state = await response.json();
        setEngineState(state);
      }
    } catch (error) {
      setIsConnected(false);
    }
  };

  const compileAndExecute = useCallback(async (code: string) => {
    try {
      // Compile VORAX-L code
      const compileResponse = await fetch('/api/vorax/compile', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ code })
      });

      const compileResult = await compileResponse.json();

      if (!compileResult.success) {
        throw new Error(compileResult.error);
      }

      // Execute compiled instructions
      const executeResponse = await fetch('/api/vorax/execute', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ 
          instructions: compileResult.instructions,
          maxTicks: 100
        })
      });

      const executeResult = await executeResponse.json();

      if (executeResult.success) {
        setEngineState(prev => ({
          ...prev,
          currentTick: executeResult.ticksExecuted,
          energyBudget: 1000 - executeResult.energyUsed
        }));
      }

      return {
        compilation: compileResult,
        execution: executeResult
      };
    } catch (error) {
      throw error;
    }
  }, []);

  const executeOperation = useCallback(async (operation: VoraxOperation) => {
    try {
      const response = await fetch('/api/execute/vorax-operation', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ operation })
      });

      const result = await response.json();

      // Update local state
      setEngineState(prev => ({
        ...prev,
        currentTick: prev.currentTick + 1
      }));

      return result;
    } catch (error) {
      throw error;
    }
  }, []);

  const resetEngine = useCallback(async () => {
    try {
      const response = await fetch('/api/vorax/reset', {
        method: 'POST'
      });

      if (response.ok) {
        const newState = await response.json();
        setEngineState(newState);
      }
    } catch (error) {
      console.error('Failed to reset engine:', error);
    }
  }, []);

  const getEngineState = useCallback(() => engineState, [engineState]);

  return {
    engine: engineState,
    isConnected,
    compileAndExecute,
    executeOperation,
    resetEngine,
    getEngineState,
    checkConnection
  };
}