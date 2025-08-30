import { useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select";
import { useMutation } from "@tanstack/react-query";
import { apiRequest } from "@/lib/queryClient";
import { useToast } from "@/hooks/use-toast";
import type { LUMGroup } from "@shared/schema";

const OPERATION_TYPES = [
  { value: 'fusion', label: 'Fusion (⧉)', symbol: '⧉', description: 'Combine deux groupes en un seul' },
  { value: 'split', label: 'Répartition (⇅)', symbol: '⇅', description: 'Distribue équitablement' },
  { value: 'cycle', label: 'Cycle (⟲)', symbol: '⟲', description: 'Retour à l\'état précédent' },
  { value: 'flow', label: 'Flux (→)', symbol: '→', description: 'Déplacement spatial' },
];

export default function LumOperations() {
  const [selectedOperation, setSelectedOperation] = useState<string>('fusion');
  const [operationResult, setOperationResult] = useState<any>(null);
  const { toast } = useToast();

  const executeMutation = useMutation({
    mutationFn: async (operation: string) => {
      // Sample input groups for demonstration
      const inputGroups = [
        {
          id: 'group1',
          lums: [
            { presence: 1, structureType: 'linear', position: { x: 0, y: 0 } },
            { presence: 1, structureType: 'linear', position: { x: 20, y: 0 } },
          ],
          groupType: 'linear',
        },
        {
          id: 'group2',
          lums: [
            { presence: 1, structureType: 'linear', position: { x: 0, y: 0 } },
            { presence: 1, structureType: 'linear', position: { x: 20, y: 0 } },
            { presence: 1, structureType: 'linear', position: { x: 40, y: 0 } },
          ],
          groupType: 'linear',
        },
      ];

      const parameters = operation === 'split' ? { zones: 2 } : operation === 'cycle' ? { modulo: 3 } : {};

      const response = await apiRequest("POST", "/api/execute/vorax-operation", {
        operation,
        inputGroups,
        parameters,
      });
      return response.json();
    },
    onSuccess: (data) => {
      setOperationResult(data.result);
      toast({
        title: "Opération exécutée",
        description: `L'opération ${selectedOperation} a été appliquée avec succès`,
      });
    },
    onError: () => {
      toast({
        title: "Erreur d'exécution",
        description: "Impossible d'exécuter l'opération VORAX",
        variant: "destructive",
      });
    },
  });

  const currentOp = OPERATION_TYPES.find(op => op.value === selectedOperation);

  const renderResult = () => {
    if (!operationResult) return null;

    if (Array.isArray(operationResult)) {
      // Split operation result
      return (
        <div className="space-y-2">
          {operationResult.map((group: LUMGroup, index: number) => (
            <div key={index} className="flex items-center">
              <span className="text-xs text-muted-foreground mr-2">Zone {index + 1}:</span>
              <div className="lum-group">
                {group.lums.map((lum, lumIndex) => (
                  <span key={lumIndex} className="lum-dot"></span>
                ))}
              </div>
            </div>
          ))}
        </div>
      );
    } else {
      // Single group result
      return (
        <div className="lum-group">
          {operationResult.lums?.map((lum: any, index: number) => (
            <span key={index} className="lum-dot"></span>
          ))}
        </div>
      );
    }
  };

  return (
    <Card>
      <CardHeader>
        <CardTitle className="flex items-center">
          <div className="w-6 h-6 bg-secondary rounded mr-3"></div>
          Opérations VORAX-SIMPLE
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-4">
        <div>
          <Label className="text-sm text-muted-foreground mb-2 block">Opération</Label>
          <Select value={selectedOperation} onValueChange={setSelectedOperation}>
            <SelectTrigger data-testid="select-operation">
              <SelectValue />
            </SelectTrigger>
            <SelectContent>
              {OPERATION_TYPES.map((op) => (
                <SelectItem key={op.value} value={op.value}>
                  {op.label}
                </SelectItem>
              ))}
            </SelectContent>
          </Select>
        </div>

        <div className="transformation-demo" data-testid="operation-demo">
          <div className="text-center space-y-2">
            {selectedOperation === 'fusion' && (
              <>
                <div className="flex items-center justify-center">
                  <div className="lum-group">
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                  </div>
                  <span className="operation-symbol">⧉</span>
                  <div className="lum-group">
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                  </div>
                </div>
                <div className="operation-symbol">↓</div>
                {operationResult ? renderResult() : (
                  <div className="lum-group opacity-50">
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                  </div>
                )}
              </>
            )}

            {selectedOperation === 'split' && (
              <>
                <div className="flex items-center justify-center">
                  <div className="lum-group">
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                  </div>
                  <span className="operation-symbol">⇅</span>
                  <span className="text-sm text-muted-foreground">2 zones</span>
                </div>
                <div className="operation-symbol">↓</div>
                {operationResult ? renderResult() : (
                  <div className="flex items-center justify-center space-x-4">
                    <div className="lum-group opacity-50">
                      <span className="lum-dot"></span>
                      <span className="lum-dot"></span>
                    </div>
                    <span className="text-muted-foreground">/</span>
                    <div className="lum-group opacity-50">
                      <span className="lum-dot"></span>
                      <span className="lum-dot"></span>
                    </div>
                  </div>
                )}
              </>
            )}

            {selectedOperation === 'cycle' && (
              <>
                <div className="flex items-center justify-center">
                  <div className="lum-group">
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                  </div>
                  <span className="operation-symbol">⟲</span>
                  <span className="text-sm text-muted-foreground">mod 3</span>
                </div>
                <div className="operation-symbol">↓</div>
                {operationResult ? renderResult() : (
                  <div className="lum-group opacity-50">
                    <span className="lum-dot"></span>
                  </div>
                )}
              </>
            )}

            {selectedOperation === 'flow' && (
              <>
                <div className="flex items-center justify-center">
                  <div className="lum-group">
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                  </div>
                  <span className="operation-symbol">→</span>
                  <span className="text-sm text-muted-foreground">Zone B</span>
                </div>
                <div className="operation-symbol">↓</div>
                {operationResult ? renderResult() : (
                  <div className="text-sm text-muted-foreground opacity-50">
                    Transfert vers Zone B
                  </div>
                )}
              </>
            )}
          </div>
        </div>

        {currentOp && (
          <div className="text-xs text-muted-foreground text-center">
            {currentOp.description}
          </div>
        )}

        <Button
          className="w-full"
          variant="secondary"
          onClick={() => executeMutation.mutate(selectedOperation)}
          disabled={executeMutation.isPending}
          data-testid="button-execute-operation"
        >
          {executeMutation.isPending ? "Exécution..." : "Exécuter l'Opération"}
        </Button>
      </CardContent>
    </Card>
  );
}
