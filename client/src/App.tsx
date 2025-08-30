import { Router, Route } from "wouter";
import IDEPage from "./pages/ide";
import NotFoundPage from "./pages/not-found";

function App() {
  return (
    <Router>
      <Route path="/" component={IDEPage} />
      <Route path="/ide" component={IDEPage} />
      <Route component={NotFoundPage} />
    </Router>
  );
}

export default App;
