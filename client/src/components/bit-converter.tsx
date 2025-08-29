import { useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import { useMutation } from "@tanstack/react-query";
import { apiRequest } from "@/lib/queryClient";
import { useToast } from "@/hooks/use-toast";
import type { LUM } from "@shared/schema";

export default function BitConverter() {
  const [inputBits, setInputBits] = useState("1101");
  const [outputLums, setOutputLums] = useState<LUM[]>([]);
  const { toast } = useToast();

  const convertMutation = useMutation({
    mutationFn: async (bits: string) => {
      const response = await apiRequest("POST", "/api/convert/bit-to-lum", { bits });
      return response.json();
    },
    onSuccess: (data) => {
      setOutputLums(data.outputLums);
      toast({
        title: "Conversion réussie",
        description: `${data.inputBits} converti en ${data.outputLums.length} LUMS`,
      });
    },
    onError: () => {
      toast({
        title: "Erreur de conversion",
        description: "Veuillez vérifier votre entrée binaire",
        variant: "destructive",
      });
    },
  });

  const handleConvert = () => {
    if (!/^[01]+$/.test(inputBits)) {
      toast({
        title: "Entrée invalide",
        description: "Veuillez entrer uniquement des 0 et des 1",
        variant: "destructive",
      });
      return;
    }
    convertMutation.mutate(inputBits);
  };

  const handleInputChange = (value: string) => {
    // Filter only 0 and 1
    const filtered = value.replace(/[^01]/g, '');
    setInputBits(filtered);
  };

  return (
    <Card>
      <CardHeader>
        <CardTitle className="flex items-center">
          <div className="w-6 h-6 bg-primary rounded mr-3"></div>
          Convertisseur Bit → LUM
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-4">
        <div>
          <Label htmlFor="binary-input" className="text-sm text-muted-foreground mb-2 block">
            Entrée Binaire
          </Label>
          <Input
            id="binary-input"
            type="text"
            value={inputBits}
            onChange={(e) => handleInputChange(e.target.value)}
            placeholder="Entrez des bits (ex: 1101)"
            className="font-mono"
            data-testid="input-binary"
          />
        </div>

        <div className="text-center py-2">
          <div className="transformation-glow inline-block px-4 py-2 rounded-lg text-sm font-medium bg-gradient-to-r from-primary to-secondary text-white">
            TRANSFORMATION
          </div>
        </div>

        <div>
          <Label className="text-sm text-muted-foreground mb-2 block">Sortie LUMS</Label>
          <div className="transformation-demo" data-testid="output-lums">
            {outputLums.length > 0 ? (
              <div className="flex items-center justify-center flex-wrap gap-2">
                {outputLums.map((lum, index) => (
                  <div key={index} className="flex items-center">
                    {lum.presence === 1 ? (
                      <div className="lum-group">
                        <span className="lum-dot"></span>
                      </div>
                    ) : (
                      <div className="w-6 h-6 flex items-center justify-center">
                        <div className="w-3 h-3 border border-muted-foreground rounded-full opacity-30"></div>
                      </div>
                    )}
                    {index < outputLums.length - 1 && inputBits[index] !== inputBits[index + 1] && (
                      <span className="mx-2 text-muted-foreground">⦿</span>
                    )}
                  </div>
                ))}
              </div>
            ) : (
              <div className="text-center text-muted-foreground">
                <div className="mb-2 text-sm">Résultat LUMS apparaîtra ici</div>
                <div className="flex items-center justify-center">
                  <div className="lum-group opacity-30">
                    <span className="lum-dot opacity-50"></span>
                    <span className="lum-dot opacity-50"></span>
                    <span className="lum-dot opacity-50"></span>
                  </div>
                </div>
              </div>
            )}
          </div>
        </div>

        <Button
          className="w-full"
          onClick={handleConvert}
          disabled={convertMutation.isPending || !inputBits}
          data-testid="button-convert"
        >
          {convertMutation.isPending ? "Conversion..." : "Convertir"}
        </Button>
      </CardContent>
    </Card>
  );
}
