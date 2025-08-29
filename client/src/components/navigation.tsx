import { Link, useLocation } from "wouter";
import { useDarkMode } from "@/hooks/use-dark-mode";
import { Button } from "@/components/ui/button";
import { Moon, Sun } from "lucide-react";

export default function Navigation() {
  const [location] = useLocation();
  const { isDarkMode, toggleDarkMode } = useDarkMode();

  return (
    <nav className="sticky top-0 z-50 bg-card/50 backdrop-blur-sm border-b border-border">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex justify-between items-center h-16">
          <div className="flex items-center space-x-4">
            <div className="w-8 h-8 bg-primary rounded-lg flex items-center justify-center">
              <span className="text-primary-foreground font-bold text-lg">L</span>
            </div>
            <Link href="/">
              <h1 className="text-xl font-bold text-foreground cursor-pointer">LUMS</h1>
            </Link>
            <span className="text-muted-foreground text-sm">Système Révolutionnaire</span>
          </div>
          
          <div className="flex items-center space-x-6">
            <Link href="/">
              <span 
                className={`text-sm cursor-pointer transition-colors ${
                  location === '/' ? 'text-foreground' : 'text-muted-foreground hover:text-foreground'
                }`}
                data-testid="link-home"
              >
                Accueil
              </span>
            </Link>
            
            <Link href="/documentation">
              <span 
                className={`text-sm cursor-pointer transition-colors ${
                  location === '/documentation' ? 'text-foreground' : 'text-muted-foreground hover:text-foreground'
                }`}
                data-testid="link-documentation"
              >
                Documentation
              </span>
            </Link>
            
            <Button
              variant="outline"
              size="sm"
              onClick={toggleDarkMode}
              data-testid="button-toggle-theme"
            >
              {isDarkMode ? <Sun className="w-4 h-4" /> : <Moon className="w-4 h-4" />}
            </Button>
          </div>
        </div>
      </div>
    </nav>
  );
}
