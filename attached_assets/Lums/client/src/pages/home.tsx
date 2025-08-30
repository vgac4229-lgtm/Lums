import BitConverter from "@/components/bit-converter";
import LumOperations from "@/components/lum-operations";
import VoraxInterpreter from "@/components/vorax-interpreter";
import CodeGenerator from "@/components/code-generator";
import LumVisualizer from "@/components/lum-visualizer";
import { Button } from "@/components/ui/button";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Link } from "wouter";

export default function Home() {
  return (
    <div className="min-h-screen">
      {/* Hero Section */}
      <section className="relative overflow-hidden bg-gradient-to-br from-primary/10 via-background to-secondary/10">
        <div className="absolute inset-0 spatial-grid opacity-30"></div>
        
        <div className="relative max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-24">
          <div className="text-center">
            <h1 className="text-5xl font-bold text-foreground mb-6">
              LUMS <span className="text-primary">(Lumaire)</span>
              <br />
              <span className="text-3xl text-muted-foreground">Révolution du Calcul Perceptif</span>
            </h1>
            <p className="text-xl text-muted-foreground mb-8 max-w-3xl mx-auto">
              Un système de calcul post-numérique basé sur les impulsions visuelles, 
              remplaçant les bits traditionnels par des unités perceptives appelées LUMS.
            </p>
            
            {/* Visual demonstration */}
            <Card className="max-w-2xl mx-auto mb-8">
              <CardHeader>
                <CardTitle className="text-lg">Démonstration Visuelle</CardTitle>
              </CardHeader>
              <CardContent className="space-y-4">
                <div className="flex items-center justify-center">
                  <span className="text-muted-foreground mr-4">Bits traditionnels:</span>
                  <code className="bg-muted px-3 py-1 rounded text-sm font-mono">1101</code>
                </div>
                <div className="operation-symbol">↓</div>
                <div className="flex items-center justify-center">
                  <span className="text-muted-foreground mr-4">LUMS équivalent:</span>
                  <div className="lum-group">
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                    <span className="lum-dot"></span>
                  </div>
                  <span className="mx-2 text-muted-foreground">⦿</span>
                  <div className="lum-group">
                    <span className="lum-dot"></span>
                  </div>
                </div>
              </CardContent>
            </Card>

            <div className="flex justify-center space-x-4">
              <Button size="lg" data-testid="button-start-demo">
                Commencer la Démonstration
              </Button>
              <Link href="/documentation">
                <Button variant="secondary" size="lg" data-testid="button-view-docs">
                  Voir la Documentation
                </Button>
              </Link>
            </div>
          </div>
        </div>
      </section>

      {/* Concept Overview */}
      <section className="py-16 bg-muted/30">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="text-center mb-12">
            <h2 className="text-3xl font-bold text-foreground mb-4">Le Concept VORAX-SIMPLE</h2>
            <p className="text-xl text-muted-foreground max-w-3xl mx-auto">
              Un paradigme révolutionnaire qui abandonne les mathématiques traditionnelles 
              au profit d'un système perceptif et intuitif.
            </p>
          </div>

          <div className="grid md:grid-cols-2 lg:grid-cols-3 gap-8">
            <Card className="fusion-animation">
              <CardHeader>
                <div className="w-12 h-12 bg-secondary/20 rounded-lg flex items-center justify-center mb-4">
                  <span className="lum-dot bg-secondary"></span>
                </div>
                <CardTitle>Unité LUM</CardTitle>
              </CardHeader>
              <CardContent>
                <p className="text-muted-foreground mb-4">
                  Une impulsion unique de présence, sans valeur numérique abstraite. 
                  Chaque • représente une existence pure.
                </p>
                <div className="flex items-center space-x-2">
                  <span className="lum-dot"></span>
                  <span className="lum-dot"></span>
                  <span className="lum-dot"></span>
                  <span className="text-muted-foreground text-sm">= 3 présences</span>
                </div>
              </CardContent>
            </Card>

            <Card className="fusion-animation">
              <CardHeader>
                <div className="w-12 h-12 bg-primary/20 rounded-lg flex items-center justify-center mb-4">
                  <span className="operation-symbol text-xl">⧉</span>
                </div>
                <CardTitle>Transformations Naturelles</CardTitle>
              </CardHeader>
              <CardContent>
                <p className="text-muted-foreground mb-4">
                  Pas d'opérateurs mathématiques. Utilise des gestes organiques : 
                  fusion, répartition, cycle, flux.
                </p>
                <div className="text-sm space-y-1">
                  <div>⧉ Fusion • ⇅ Répartition</div>
                  <div>⟲ Cycle • → Flux</div>
                </div>
              </CardContent>
            </Card>

            <Card className="fusion-animation">
              <CardHeader>
                <div className="w-12 h-12 bg-accent/20 rounded-lg flex items-center justify-center mb-4">
                  <span className="operation-symbol text-xl">∞</span>
                </div>
                <CardTitle>Structures Spatiales</CardTitle>
              </CardHeader>
              <CardContent>
                <p className="text-muted-foreground mb-4">
                  Les LUMS existent dans l'espace libre : lignes, cercles, grappes, nœuds. 
                  La structure influence le sens.
                </p>
                <div className="space-y-2">
                  <div className="text-sm">•••• (ligne)</div>
                  <div className="text-sm">⦿(•••) (grappe)</div>
                  <div className="text-sm">∞(••) (nœud)</div>
                </div>
              </CardContent>
            </Card>
          </div>
        </div>
      </section>

      {/* Interactive Demo */}
      <section className="py-16">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="text-center mb-12">
            <h2 className="text-3xl font-bold text-foreground mb-4">Démonstration Interactive</h2>
            <p className="text-xl text-muted-foreground">
              Explorez les concepts LUMS en temps réel
            </p>
          </div>

          <div className="grid lg:grid-cols-2 gap-8 mb-8">
            <BitConverter />
            <LumOperations />
          </div>

          <VoraxInterpreter />
        </div>
      </section>

      {/* Visualization Playground */}
      <section className="py-16 bg-muted/20">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <h2 className="text-3xl font-bold text-center mb-12">Laboratoire de Visualisation</h2>
          <LumVisualizer />
        </div>
      </section>

      {/* Code Implementation */}
      <section className="py-16">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="text-center mb-12">
            <h2 className="text-3xl font-bold text-foreground mb-4">Générateur de Code C</h2>
            <p className="text-xl text-muted-foreground">
              Encodeur/Décodeur C et structures de données LUMS
            </p>
          </div>
          <CodeGenerator />
        </div>
      </section>
    </div>
  );
}
