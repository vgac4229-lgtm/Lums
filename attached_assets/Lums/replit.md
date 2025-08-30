# Overview

This is a full-stack LUMS (Lumaire) application built with Express.js, React, and TypeScript. LUMS represents a revolutionary post-numerical computing paradigm based on visual perception units rather than traditional bits. The system includes a bit-to-LUM converter, VORAX interpreter for LUM operations, code generator for C implementations, and real-time visualization capabilities. The application provides both a web interface for experimenting with LUMS concepts and a REST API for programmatic access to the core functionality.

# User Preferences

Preferred communication style: Simple, everyday language.

# System Architecture

## Frontend Architecture
The client uses a modern React setup with TypeScript and Vite for development. The UI is built with shadcn/ui components providing a consistent design system with Tailwind CSS for styling. The application follows a component-based architecture with dedicated components for:

- **Bit Converter**: Transforms binary strings into LUM representations
- **LUM Operations**: Implements VORAX operations (fusion, split, cycle, flow)
- **VORAX Interpreter**: Parses and executes VORAX language code
- **Code Generator**: Creates C code implementations for LUM operations
- **LUM Visualizer**: Provides real-time canvas-based visualization

State management uses React Query (TanStack Query) for server state synchronization and local React state for UI interactions. The routing is handled by Wouter, providing a lightweight client-side routing solution.

## Backend Architecture
The server is built with Express.js and TypeScript, following a layered architecture pattern. The main components include:

- **Routes Layer**: RESTful API endpoints for sessions, operations, and conversions
- **Storage Layer**: Abstracted storage interface with in-memory implementation
- **Schema Layer**: Shared TypeScript types and Zod validation schemas

The API provides endpoints for managing LUM sessions, executing VORAX operations, converting between bits and LUMs, and generating C code. All data validation uses Zod schemas to ensure type safety across the client-server boundary.

## Data Storage Solutions
Currently implements an in-memory storage system using Maps for rapid prototyping. The architecture includes a storage abstraction layer (IStorage interface) that can be easily swapped for persistent storage solutions. Database schema is defined using Drizzle ORM with PostgreSQL dialect, preparing for future database integration. The storage layer handles LUM sessions and operations with full CRUD capabilities.

## Authentication and Authorization Mechanisms
The current implementation does not include authentication or authorization mechanisms, focusing on the core LUMS functionality. The architecture is prepared for future authentication integration through middleware patterns already established in the Express setup.

## External Dependencies
The system uses several key external libraries:

- **@neondatabase/serverless**: PostgreSQL driver for future database integration
- **Drizzle ORM**: Type-safe database toolkit with PostgreSQL support
- **Radix UI**: Accessible component primitives for the UI layer
- **TanStack React Query**: Server state management and caching
- **Zod**: Runtime type validation and schema definition
- **Vite**: Modern build tool with HMR support
- **Tailwind CSS**: Utility-first CSS framework

The application is configured for deployment on Replit with specific build and development scripts. The Vite configuration includes Replit-specific plugins for development environment integration and error handling.