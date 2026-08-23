/**
 * Registry for runtime type lookup by name.
 * Used by deserializers to instantiate the correct concrete class for
 * polymorphic fields and for MikanRequest/Response/Event routing.
 */
export class TypeRegistry {
  private static types = new Map<string, any>();

  static register(name: string, type: any): void {
    this.types.set(name, type);
  }

  static get(name: string): any | undefined {
    return this.types.get(name);
  }

  static has(name: string): boolean {
    return this.types.has(name);
  }
}
