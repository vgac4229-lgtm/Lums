import { useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import { Textarea } from "@/components/ui/textarea";
import { parseVoraxCode } from "@/lib/vorax-parser";
import type { LUMGroup } from "@shared/schema";

const DEFAULT_CODE = `Zone A : ⦿(•••)
#graine := ⦿(••)

#graine ⧉ • → ⦿(•••)
⇾ Zone A reçoit #graine
Zone A : ⦿(•••) + ⦿(•••)
⇾ ⇅ 3 → (• / • / •)`;

export default function VoraxInterpreter() {
  const [voraxCode, setVoraxCode] = useState(DEFAULT_CODE);
  const [executionResult, setExecutionResult] = useState<any>(null);
  const [isExecuting, setIsExecuting] = useState(false);

  const executeVorax = async () => {
    setIsExecuting(true);
    try {
      // Parse the VORAX code
      const parsed = parseVoraxCode(voraxCode);
      
      // Simulate execution result
      const result = {
        zones: {
          'Zone A': {
            id: 'zoneA',
            lums: [
              { presence: 1, structureType: 'group' as const },
              { presence: 1, structureType: 'group' as const },
              { presence: 1, structureType: 'group' as const },
            ],
            groupType: 'cluster' as const,
          }
        },
        memory: {
          'graine': {
            id: 'graine',
            lums: [
              { presence: 1, structureType: 'group' as const },
              { presence: 1, structureType: 'group' as const },
              { presence: 1, structureType: 'group' as const },
            ],
            groupType: 'cluster' as const,
          }
        },
        operations: parsed.operations,
        status: 'executed'
      };
      
      setExecutionResult(result);
    } catch (error) {
      console.error('VORAX execution error:', error);
      setExecutionResult({ error: 'Erreur d\'exécution du code VORAX' });
    } finally {
      setIsExecuting(false);
    }
  };

  const renderZoneVisualization = (zone: LUMGroup) => (
    <div className="lum-group">
      {zone.lums.map((lum, index) => (
        <span key={index} className="lum-dot"></span>
      ))}
    </div>
  );

  return (
    <Card className="mt-8">
      <CardHeader>
        <CardTitle>Interpréteur VORAX-SIMPLE</CardTitle>
      </CardHeader>
      <CardContent>
        <div className="grid md:grid-cols-2 gap-6">
          <div>
            <Label className="text-sm font-medium mb-2 block">Code VORAX</Label>
            <Textarea
              value={voraxCode}
              onChange={(e) => setVoraxCode(e.target.value)}
              className="h-32 font-mono text-sm resize-none"
              placeholder="Zone A : ⦿(•••)&#10;#graine := ⦿(••)&#10;#graine ⧉ • → ⦿(•••)"
              data-testid="textarea-vorax-code"
            />
            <Button
              className="mt-2"
              onClick={executeVorax}
              disabled={isExecuting}
              data-testid="button-execute-vorax"
            >
              {isExecuting ? "Exécution..." : "Exécuter"}
            </Button>
          </div>
          
          <div>
            <Label className="text-sm font-medium mb-2 block">Résultat Visual</Label>
            <div className="border border-border rounded-md p-4 h-32 bg-muted overflow-auto" data-testid="vorax-result">
              {executionResult ? (
                executionResult.error ? (
                  <div className="text-destructive text-sm">{executionResult.error}</div>
                ) : (
                  <div className="space-y-2">
                    {Object.entries(executionResult.zones || {}).map(([zoneName, zone]) => (
                      <div key={zoneName} className="text-sm">
                        <span className="text-muted-foreground">{zoneName}: </span>
                        {renderZoneVisualization(zone as LUMGroup)}
                      </div>
                    ))}
                    {Object.entries(executionResult.memory || {}).map(([memName, mem]) => (
                      <div key={memName} className="text-sm">
                        <span className="text-muted-foreground">#{memName}: </span>
                        {renderZoneVisualization(mem as LUMGroup)}
                      </div>
                    ))}
                    <div className="text-xs text-muted-foreground mt-2">
                      → {executionResult.operations?.length || 0} opération(s) exécutée(s)
                    </div>
                  </div>
                )
              ) : (
                <div className="space-y-2">
                  <div className="text-sm">
                    <span className="text-muted-foreground">Zone A: </span>
                    <div className="lum-group inline-flex opacity-50">
                      <span className="lum-dot"></span>
                      <span className="lum-dot"></span>
                      <span className="lum-dot"></span>
                    </div>
                  </div>
                  <div className="text-xs text-muted-foreground">→ En attente d'exécution...</div>
                </div>
              )}
            </div>
          </div>
        </div>
      </CardContent>
    </Card>
  );
}
