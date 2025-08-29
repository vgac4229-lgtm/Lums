import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { Button } from "@/components/ui/button";
import { ExternalLink, Github, BookOpen } from "lucide-react";

export default function Documentation() {
  return (
    <div className="min-h-screen py-12">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="text-center mb-12">
          <h1 className="text-4xl font-bold text-foreground mb-4">Documentation LUMS</h1>
          <p className="text-xl text-muted-foreground max-w-3xl mx-auto">
            Guide complet du paradigme LUMS et du système VORAX-SIMPLE
          </p>
        </div>

        {/* Overview */}
        <section className="mb-16">
          <h2 className="text-3xl font-bold mb-8">Vue d'ensemble</h2>
          <div className="grid md:grid-cols-3 gap-6">
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center">
                  <div className="w-8 h-8 bg-primary/20 rounded-lg flex items-center justify-center mr-3">
                    <span className="text-primary">•</span>
                  </div>
                  Paradigme Fondamental
                </CardTitle>
              </CardHeader>
              <CardContent className="space-y-3 text-sm">
                <div className="p-3 bg-muted rounded-md">
                  <h4 className="font-medium mb-1">Unité LUM</h4>
                  <p className="text-muted-foreground">Impulsion de présence pure, sans valeur numérique abstraite</p>
                </div>
                <div className="p-3 bg-muted rounded-md">
                  <h4 className="font-medium mb-1">Post-Numérique</h4>
                  <p className="text-muted-foreground">Abandon des mathématiques traditionnelles au profit de la perception</p>
                </div>
                <div className="p-3 bg-muted rounded-md">
                  <h4 className="font-medium mb-1">Spatial</h4>
                  <p className="text-muted-foreground">Structure influence la signification : ligne, grappe, nœud</p>
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle className="flex items-center">
                  <div className="w-8 h-8 bg-secondary/20 rounded-lg flex items-center justify-center mr-3">
                    <span className="text-secondary">⧉</span>
                  </div>
                  Opérations VORAX
                </CardTitle>
              </CardHeader>
              <CardContent className="space-y-3">
                <div className="border-l-4 border-primary pl-3">
                  <h4 className="font-medium">⧉ Fusion</h4>
                  <p className="text-sm text-muted-foreground">Combine deux groupes en un seul</p>
                  <code className="text-xs bg-muted px-1 rounded">•• ⧉ ••• → •••••</code>
                </div>
                <div className="border-l-4 border-secondary pl-3">
                  <h4 className="font-medium">⇅ Répartition</h4>
                  <p className="text-sm text-muted-foreground">Distribue équitablement</p>
                  <code className="text-xs bg-muted px-1 rounded">•••• ⇅ 2 → •• / ••</code>
                </div>
                <div className="border-l-4 border-accent pl-3">
                  <h4 className="font-medium">⟲ Cycle</h4>
                  <p className="text-sm text-muted-foreground">Retour à l'état précédent</p>
                  <code className="text-xs bg-muted px-1 rounded">••• ⟲ → •</code>
                </div>
                <div className="border-l-4 border-destructive pl-3">
                  <h4 className="font-medium">→ Flux</h4>
                  <p className="text-sm text-muted-foreground">Déplacement spatial</p>
                  <code className="text-xs bg-muted px-1 rounded">•• → Zone B</code>
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle className="flex items-center">
                  <div className="w-8 h-8 bg-accent/20 rounded-lg flex items-center justify-center mr-3">
                    <span className="text-accent">C</span>
                  </div>
                  Implémentation
                </CardTitle>
              </CardHeader>
              <CardContent className="space-y-4">
                <div>
                  <h4 className="font-medium mb-2">1. Structure de Base</h4>
                  <div className="code-block text-xs">
                    <pre>{`typedef struct LUM {
  int presence;
  int structure_type;
  void* spatial_data;
} LUM;`}</pre>
                  </div>
                </div>
                <div>
                  <h4 className="font-medium mb-2">2. Encodage</h4>
                  <p className="text-sm text-muted-foreground">Conversion bit → LUM avec préservation spatiale</p>
                </div>
                <div>
                  <h4 className="font-medium mb-2">3. Opérations</h4>
                  <p className="text-sm text-muted-foreground">Transformations organiques sur structures LUM</p>
                </div>
              </CardContent>
            </Card>
          </div>
        </section>

        {/* Syntax Reference */}
        <section className="mb-16">
          <h2 className="text-3xl font-bold mb-8">Référence Syntaxe VORAX-SIMPLE</h2>
          <div className="grid lg:grid-cols-2 gap-8">
            <Card>
              <CardHeader>
                <CardTitle>Déclarations de Base</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div>
                    <h4 className="font-medium mb-2">Zones et Mémoire</h4>
                    <div className="code-block">
                      <pre className="text-sm">{`Zone A, B, C;           // Déclaration zones
mem buf, cache;         // Mémoire tampon
#alpha := ⦿(•••);      // Définition motif`}</pre>
                    </div>
                  </div>
                  <div>
                    <h4 className="font-medium mb-2">Structures LUM</h4>
                    <div className="code-block">
                      <pre className="text-sm">{`•••                     // Ligne de 3 LUMs
⦿(••••)                // Grappe de 4 LUMs
∞(⦿(••), ⦿(•••))      // Nœud complexe`}</pre>
                    </div>
                  </div>
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Opérations et Flux</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div>
                    <h4 className="font-medium mb-2">Transformations</h4>
                    <div className="code-block">
                      <pre className="text-sm">{`emit A += 2•;           // Ajout de LUMs
split A -> [B, C];      // Répartition
move B -> D, 1•;        // Déplacement`}</pre>
                    </div>
                  </div>
                  <div>
                    <h4 className="font-medium mb-2">Conditions et Cycles</h4>
                    <div className="code-block">
                      <pre className="text-sm">{`on (empty B) {          // Condition
  retrieve buf -> B;
}
cycle A % 3;            // Cycle modulaire`}</pre>
                    </div>
                  </div>
                </div>
              </CardContent>
            </Card>
          </div>
        </section>

        {/* Examples */}
        <section className="mb-16">
          <h2 className="text-3xl font-bold mb-8">Exemples Pratiques</h2>
          <Card>
            <CardHeader>
              <CardTitle>Comparaison Calcul Traditionnel vs LUMS</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="grid md:grid-cols-2 gap-8">
                <div>
                  <h4 className="text-lg font-medium mb-3">Addition Traditionnelle</h4>
                  <div className="space-y-4">
                    <div className="p-4 bg-muted rounded-md">
                      <h5 className="font-medium mb-2">Calcul: 2 + 3 = 5</h5>
                      <div className="text-sm text-muted-foreground">
                        Abstraction numérique avec opérateurs mathématiques
                      </div>
                      <div className="mt-2 font-mono text-sm">
                        int a = 2;<br />
                        int b = 3;<br />
                        int result = a + b; // 5
                      </div>
                    </div>
                  </div>
                </div>
                
                <div>
                  <h4 className="text-lg font-medium mb-3">Fusion LUMS</h4>
                  <div className="space-y-4">
                    <div className="p-4 bg-secondary/10 border border-secondary/20 rounded-md">
                      <h5 className="font-medium mb-2">LUMS: •• ⧉ ••• → •••••</h5>
                      <div className="text-sm text-muted-foreground mb-3">
                        Fusion de présences visuelles perceptibles
                      </div>
                      <div className="flex items-center justify-center mb-3">
                        <div className="lum-group">
                          <span className="lum-dot"></span>
                          <span className="lum-dot"></span>
                        </div>
                        <span className="operation-symbol text-sm">⧉</span>
                        <div className="lum-group">
                          <span className="lum-dot"></span>
                          <span className="lum-dot"></span>
                          <span className="lum-dot"></span>
                        </div>
                        <span className="operation-symbol text-sm">→</span>
                        <div className="lum-group">
                          <span className="lum-dot"></span>
                          <span className="lum-dot"></span>
                          <span className="lum-dot"></span>
                          <span className="lum-dot"></span>
                          <span className="lum-dot"></span>
                        </div>
                      </div>
                      <div className="code-block text-sm">
                        <pre>{`Zone A : ••
Zone B : •••
A ⧉ B → •••••`}</pre>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </CardContent>
          </Card>
        </section>

        {/* API Reference */}
        <section className="mb-16">
          <h2 className="text-3xl font-bold mb-8">Référence API C</h2>
          <div className="space-y-6">
            <Card>
              <CardHeader>
                <CardTitle>Fonctions d'Encodage/Décodage</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div>
                    <h4 className="font-medium mb-2">Encodage Bit → LUM</h4>
                    <div className="code-block">
                      <pre className="text-sm">{`LUM* encode_bit_to_lum(uint8_t input);
LUMGroup* encode_to_lum_group(uint8_t input);

// Exemple d'utilisation
uint8_t bits = 0b1101;  // 13 en décimal
LUM* lums = encode_bit_to_lum(bits);`}</pre>
                    </div>
                  </div>
                  <div>
                    <h4 className="font-medium mb-2">Décodage LUM → Bit</h4>
                    <div className="code-block">
                      <pre className="text-sm">{`uint8_t decode_lum_to_bit(LUM* lums, int count);
uint8_t decode_lum_group(LUMGroup* group);

// Exemple d'utilisation
uint8_t restored = decode_lum_to_bit(lums, 4);`}</pre>
                    </div>
                  </div>
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Opérations VORAX</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="code-block">
                  <pre className="text-sm">{`// Opération de Fusion (⧉)
LUMGroup* lum_fusion(LUMGroup* group1, LUMGroup* group2);

// Opération de Répartition (⇅)
LUMGroup** lum_split(LUMGroup* source, int zones);

// Opération de Cycle (⟲)
LUMGroup* lum_cycle(LUMGroup* source, int modulo);

// Opération de Flux (→)
void lum_flow(LUMGroup* source, Zone* target);`}</pre>
                </div>
              </CardContent>
            </Card>
          </div>
        </section>

        {/* Resources */}
        <section>
          <h2 className="text-3xl font-bold mb-8">Ressources et Liens</h2>
          <div className="grid md:grid-cols-3 gap-6">
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center">
                  <BookOpen className="w-5 h-5 mr-2" />
                  Guide de Démarrage
                </CardTitle>
              </CardHeader>
              <CardContent>
                <p className="text-sm text-muted-foreground mb-4">
                  Tutoriel complet pour commencer avec le système LUMS
                </p>
                <Button variant="outline" className="w-full" data-testid="button-guide">
                  <ExternalLink className="w-4 h-4 mr-2" />
                  Accéder au Guide
                </Button>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle className="flex items-center">
                  <Github className="w-5 h-5 mr-2" />
                  Code Source
                </CardTitle>
              </CardHeader>
              <CardContent>
                <p className="text-sm text-muted-foreground mb-4">
                  Implémentation complète et exemples de code
                </p>
                <div className="space-y-2">
                  <Button variant="outline" size="sm" className="w-full" data-testid="button-onramp">
                    <ExternalLink className="w-3 h-3 mr-2" />
                    Onramp
                  </Button>
                  <Button variant="outline" size="sm" className="w-full" data-testid="button-planckforth">
                    <ExternalLink className="w-3 h-3 mr-2" />
                    PlanckForth
                  </Button>
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Communauté</CardTitle>
              </CardHeader>
              <CardContent>
                <p className="text-sm text-muted-foreground mb-4">
                  Rejoignez la communauté des développeurs LUMS
                </p>
                <div className="space-y-2">
                  <Button variant="outline" size="sm" className="w-full" data-testid="button-forum">
                    Forum Discussion
                  </Button>
                  <Button variant="outline" size="sm" className="w-full" data-testid="button-discord">
                    Discord
                  </Button>
                  <Button variant="outline" size="sm" className="w-full" data-testid="button-contribute">
                    Contribuer
                  </Button>
                </div>
              </CardContent>
            </Card>
          </div>
        </section>
      </div>
    </div>
  );
}
