/**
 * Base class for polymorphic struct types
 * Used as a marker for types that can be stored in PolymorphicObject
 */
export class PolymorphicStruct {
  // Marker class - all serializable types can be polymorphic
}

/**
 * Container for polymorphic object instances
 * Stores a runtime type ID and the actual object instance
 */
export class PolymorphicObject {
  private _runtimeClassName: string = '';
  private _instance: PolymorphicStruct | null = null;

  get runtimeClassName(): string {
    return this._runtimeClassName;
  }

  get instance(): PolymorphicStruct | null {
    return this._instance;
  }

  constructor(instance?: PolymorphicStruct, className?: string) {
    if (instance && className !== undefined) {
      this.setInstance(instance, className);
    }
  }

  public setInstance(instance: PolymorphicStruct, className: string): void {
    this._runtimeClassName = className;
    this._instance = instance;
  }

  public toJSON(): any {
    return {
      class_name: this._runtimeClassName,
      value: this._instance
    };
  }

  public static fromJSON(json: any): PolymorphicObject {
    const obj = new PolymorphicObject();
    if (json.class_name) {
      obj._runtimeClassName = json.class_name;
    }
    // Extract the instance from the remaining fields
    const { class_name, ...instanceData } = json;
    obj._instance = instanceData as PolymorphicStruct;
    return obj;
  }
}
