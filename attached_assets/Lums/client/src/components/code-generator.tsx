import { useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select";
import { useMutation } from "@tanstack/react-query";
import { apiRequest } from "@/lib/queryClient";
import { useToast } from "@/hooks/use-toast";
import { Download } from "lucide-react";

const DATA_TYPES = [
  { value: 'uint8_t', label: 'uint8_t (8 bits)' },
  { value: 'uint16_t', label: 'uint16_t (16 bits)' },
  { value: 'uint32_t', label: 'uint32_t (32 bits)' },
  { value: 'uint64_t', label: 'uint64_t (64 bits)' },
];

const OUTPUT_MODES = [
  { value: 'struct', label: 'Structure LUMS' },
  { value: 'array', label: 'Tableau de présences' },
  { value: 'spatial', label: 'Format spatial' },
];

const CODE_TYPES = [
  { value: 'encoder', label: 'Encodeur (Bit → LUM)' },
  { value: 'decoder', label: 'Décodeur (LUM → Bit)' },
  { value: 'operations', label: 'Opérations VORAX' },
];

export default function CodeGenerator() {
  const [codeType, setCodeType] = useState('encoder');
  const [dataType, setDataType] = useState('uint8_t');
  const [outputMode, setOutputMode] = useState('struct');
  const [generatedCode, setGeneratedCode] = useState('');
  const { toast } = useToast();

  const generateMutation = useMutation({
    mutationFn: async () => {
      const response = await apiRequest("POST", "/api/generate/c-code", {
        codeType,
        parameters: { dataType, outputMode },
      });
      return response.json();
    },
    onSuccess: (data) => {
      setGeneratedCode(data.generatedCode);
      toast({
        title: "Code généré avec succès",
        description: `${codeType} généré pour ${dataType}`,
      });
    },
    onError: () => {
      toast({
        title: "Erreur de génération",
        description: "Impossible de générer le code C",
        variant: "destructive",
      });
    },
  });

  const downloadCode = () => {
    if (!generatedCode) return;

    const filename = `lums_${codeType}_${dataType}.c`;
    const blob = new Blob([generatedCode], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);

    toast({
      title: "Téléchargement démarré",
      description: `Fichier ${filename} téléchargé`,
    });
  };

  return (
    <div className="grid lg:grid-cols-2 gap-8">
      <Card>
        <CardHeader>
          <CardTitle>Configuration du Générateur</CardTitle>
        </CardHeader>
        <CardContent className="space-y-4">
          <div>
            <Label className="text-sm text-muted-foreground mb-2 block">Type de Code</Label>
            <Select value={codeType} onValueChange={setCodeType}>
              <SelectTrigger data-testid="select-code-type">
                <SelectValue />
              </SelectTrigger>
              <SelectContent>
                {CODE_TYPES.map((type) => (
                  <SelectItem key={type.value} value={type.value}>
                    {type.label}
                  </SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>

          <div>
            <Label className="text-sm text-muted-foreground mb-2 block">Type de Données</Label>
            <Select value={dataType} onValueChange={setDataType}>
              <SelectTrigger data-testid="select-data-type">
                <SelectValue />
              </SelectTrigger>
              <SelectContent>
                {DATA_TYPES.map((type) => (
                  <SelectItem key={type.value} value={type.value}>
                    {type.label}
                  </SelectItem>
                ))}
              </SelectContent>
            </Select>
          </div>

          {codeType === 'encoder' && (
            <div>
              <Label className="text-sm text-muted-foreground mb-2 block">Mode de Sortie</Label>
              <Select value={outputMode} onValueChange={setOutputMode}>
                <SelectTrigger data-testid="select-output-mode">
                  <SelectValue />
                </SelectTrigger>
                <SelectContent>
                  {OUTPUT_MODES.map((mode) => (
                    <SelectItem key={mode.value} value={mode.value}>
                      {mode.label}
                    </SelectItem>
                  ))}
                </SelectContent>
              </Select>
            </div>
          )}

          <Button
            className="w-full"
            onClick={() => generateMutation.mutate()}
            disabled={generateMutation.isPending}
            data-testid="button-generate-code"
          >
            {generateMutation.isPending ? "Génération..." : `Générer ${CODE_TYPES.find(t => t.value === codeType)?.label}`}
          </Button>
        </CardContent>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle>Code C Généré</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="bg-muted border border-border rounded-lg p-4 h-64 overflow-auto" data-testid="generated-code">
            {generatedCode ? (
              <pre className="text-sm font-mono text-foreground whitespace-pre-wrap">
                {generatedCode}
              </pre>
            ) : (
              <div className="flex items-center justify-center h-full text-muted-foreground">
                <div className="text-center">
                  <div className="mb-2">Aucun code généré</div>
                  <div className="text-xs">Configurez les paramètres et cliquez sur "Générer"</div>
                </div>
              </div>
            )}
          </div>
          
          <Button
            className="mt-4 w-full"
            variant="outline"
            onClick={downloadCode}
            disabled={!generatedCode}
            data-testid="button-download-code"
          >
            <Download className="w-4 h-4 mr-2" />
            Télécharger le Code
          </Button>
        </CardContent>
      </Card>
    </div>
  );
}
