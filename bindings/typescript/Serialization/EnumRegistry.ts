/**
 * Registry for TypeScript enum types, enabling string-name ↔ integer-value conversion
 * during serialization/deserialization.
 */
export class EnumRegistry {
  private static enums = new Map<string, any>();

  static register(name: string, enumObj: any): void {
    this.enums.set(name, enumObj);
  }

  static get(name: string): any | undefined {
    return this.enums.get(name);
  }

  static has(name: string): boolean {
    return this.enums.has(name);
  }

  /**
   * Returns all options for a registered enum as label/value pairs.
   * Filters out the reverse-mapping entries that TypeScript numeric enums generate.
   */
  static getOptions(enumTypeName: string): { label: string; value: number }[] {
    const enumObj = this.enums.get(enumTypeName);
    if (!enumObj) return [];
    return Object.entries(enumObj)
      .filter(([, v]) => typeof v === 'number')
      .map(([label, value]) => ({ label, value: value as number }));
  }

  /**
   * Convert a string enum key (e.g. "NONE") to its integer value (e.g. 0).
   * Returns null if the enum type is unknown or the key is not found.
   */
  static stringToInt(enumTypeName: string, stringValue: string): number | null {
    const enumObj = this.enums.get(enumTypeName);
    if (!enumObj) return null;
    const value = enumObj[stringValue];
    if (typeof value === 'number') return value;
    return null;
  }

  /**
   * Convert an integer enum value (e.g. 0) back to its string key (e.g. "NONE").
   * Returns null if the enum type is unknown or the value is not found.
   */
  static intToString(enumTypeName: string, intValue: number): string | null {
    const enumObj = this.enums.get(enumTypeName);
    if (!enumObj) return null;
    const label = enumObj[intValue];
    if (typeof label === 'string') return label;
    return null;
  }
}
