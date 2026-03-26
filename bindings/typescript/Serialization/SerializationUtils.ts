import { PolymorphicObject, PolymorphicStruct } from '../PolymorphicObject';

/**
 * Represents a value accessor that can get/set values on objects
 * Similar to C# ValueAccessor
 */
export class ValueAccessor {
  private instance: any;
  private fieldName: string | null;
  private type: any;
  private typeName: string;

  constructor(instance: any, type: any, fieldName: string | null = null) {
    this.instance = instance;
    this.fieldName = fieldName;
    this.type = type;
    this.typeName = type?.name || 'unknown';
  }

  get valueField(): string | null {
    return this.fieldName;
  }

  get valueType(): any {
    return this.type;
  }

  get valueName(): string {
    return this.fieldName || this.typeName;
  }

  get valueInstance(): any {
    return this.instance;
  }

  ensureValueAllocated(): void {
    if (this.fieldName && !this.instance[this.fieldName]) {
      this.instance[this.fieldName] = new this.type();
    } else if (!this.instance) {
      this.instance = new this.type();
    }
  }

  getValueObject(): any {
    if (this.fieldName) {
      return this.instance[this.fieldName];
    }
    return this.instance;
  }

  setValueObject(value: any): void {
    if (this.fieldName) {
      this.instance[this.fieldName] = value;
    } else {
      this.instance = value;
    }
  }

  getValue<T>(): T {
    if (this.fieldName) {
      return this.instance[this.fieldName] as T;
    }
    return this.instance as T;
  }

  setValue<T>(value: T): void {
    if (this.fieldName) {
      this.instance[this.fieldName] = value;
    } else {
      this.instance = value;
    }
  }
}

/**
 * Visitor interface for serialization/deserialization
 */
export interface IVisitor {
  visitClass(accessor: ValueAccessor): void;
  visitList(accessor: ValueAccessor): void;
  visitDictionary(accessor: ValueAccessor): void;
  visitPolymorphicObject(accessor: ValueAccessor): void;
  visitEnum(accessor: ValueAccessor): void;
  visitBool(accessor: ValueAccessor): void;
  visitByte(accessor: ValueAccessor): void;
  visitUByte(accessor: ValueAccessor): void;
  visitShort(accessor: ValueAccessor): void;
  visitUShort(accessor: ValueAccessor): void;
  visitInt(accessor: ValueAccessor): void;
  visitUInt(accessor: ValueAccessor): void;
  visitLong(accessor: ValueAccessor): void;
  visitULong(accessor: ValueAccessor): void;
  visitFloat(accessor: ValueAccessor): void;
  visitDouble(accessor: ValueAccessor): void;
  visitString(accessor: ValueAccessor): void;
}

/**
 * Metadata for serializable fields
 */
export interface FieldMetadata {
  name: string;
  type: string;
  isArray?: boolean;
  isMap?: boolean;
  elementType?: string;
  keyType?: string;
  valueType?: string;
}

/**
 * Get serialization metadata for a class, walking the prototype chain so that
 * inherited fields from parent classes are included (parent fields first).
 */
export function getSerializationMetadata(instance: any): FieldMetadata[] {
  if (!instance || !instance.constructor) {
    return [];
  }

  // Collect metadata arrays from each class in the hierarchy, stopping at Object.
  // We push each layer to the front so parent class fields come first.
  const chain: FieldMetadata[][] = [];
  let ctor = instance.constructor;
  while (ctor && ctor !== Object) {
    if (Object.prototype.hasOwnProperty.call(ctor, '__serializationMetadata')) {
      chain.unshift(ctor.__serializationMetadata);
    }
    ctor = Object.getPrototypeOf(ctor);
  }

  return chain.flat();
}

/**
 * Visit an object and all its fields
 */
export function visitObject(instance: any, instanceType: any, visitor: IVisitor): void {
  const metadata = getSerializationMetadata(instance);

  for (const field of metadata) {
    const accessor = new ValueAccessor(instance, instanceType, field.name);
    // Store the field metadata on the accessor for use by visitors
    (accessor as any).fieldMetadata = field;
    visitField(accessor, field, visitor);
  }
}

/**
 * Visit a single field
 */
export function visitField(accessor: ValueAccessor, field: FieldMetadata, visitor: IVisitor): void {
  visitValue(accessor, field, visitor);
}

/**
 * Visit a value based on its type
 */
export function visitValue(accessor: ValueAccessor, field: FieldMetadata, visitor: IVisitor): void {
  const typeName = field.type;

  if (field.isArray) {
    visitor.visitList(accessor);
  } else if (field.isMap) {
    visitor.visitDictionary(accessor);
  } else if (typeName === 'PolymorphicObject') {
    visitor.visitPolymorphicObject(accessor);
  } else if (typeName === 'string') {
    visitor.visitString(accessor);
  } else if (typeName === 'boolean') {
    visitor.visitBool(accessor);
  } else if (typeName === 'int8') {
    visitor.visitByte(accessor);
  } else if (typeName === 'uint8') {
    visitor.visitUByte(accessor);
  } else if (typeName === 'int16') {
    visitor.visitShort(accessor);
  } else if (typeName === 'uint16') {
    visitor.visitUShort(accessor);
  } else if (typeName === 'int32' || typeName === 'number') {
    visitor.visitInt(accessor);
  } else if (typeName === 'uint32') {
    visitor.visitUInt(accessor);
  } else if (typeName === 'int64' || typeName === 'bigint') {
    visitor.visitLong(accessor);
  } else if (typeName === 'uint64') {
    visitor.visitULong(accessor);
  } else if (typeName === 'float') {
    visitor.visitFloat(accessor);
  } else if (typeName === 'double') {
    visitor.visitDouble(accessor);
  } else if (typeName.startsWith('enum:')) {
    visitor.visitEnum(accessor);
  } else {
    // Assume it's a class/struct
    visitor.visitClass(accessor);
  }
}

/**
 * Helper to get the class ID from a type
 */
export function getMikanClassId(type: any): bigint {
  if (type && type.classId !== undefined) {
    return type.classId;
  }
  return 0n;
}
