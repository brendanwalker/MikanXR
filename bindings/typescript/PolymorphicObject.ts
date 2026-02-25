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
  private _runtimeClassId: bigint = 0n;
  private _instance: PolymorphicStruct | null = null;

  get runtimeClassId(): bigint {
    return this._runtimeClassId;
  }

  get instance(): PolymorphicStruct | null {
    return this._instance;
  }

  constructor(instance?: PolymorphicStruct, classId?: bigint) {
    if (instance && classId !== undefined) {
      this.setInstance(instance, classId);
    }
  }

  public setInstance(instance: PolymorphicStruct, classId: bigint): void {
    this._runtimeClassId = classId;
    this._instance = instance;
  }

  public toJSON(): any {
    return {
      runtimeClassId: this._runtimeClassId.toString(),
      instance: this._instance
    };
  }

  public static fromJSON(json: any): PolymorphicObject {
    const obj = new PolymorphicObject();
    if (json.runtimeClassId) {
      obj._runtimeClassId = BigInt(json.runtimeClassId);
    }
    obj._instance = json.instance;
    return obj;
  }
}
