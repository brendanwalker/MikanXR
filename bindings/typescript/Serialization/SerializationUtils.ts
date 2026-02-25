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
 * Get serialization metadata for a class
 */
export function getSerializationMetadata(instance: any): FieldMetadata[] {
  if (!instance || !instance.constructor || !instance.constructor.__serializationMetadata) {
    return [];
  }
  return instance.constructor.__serializationMetadata;
}

/**
 * Visit an object and all its fields
 */
export function visitObject(instance: any, instanceType: any, visitor: IVisitor): void {
  const metadata = getSerializationMetadata(instance);

  for (const field of metadata) {
    const accessor = new ValueAccessor(instance, instanceType, field.name);
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
  } else if (typeName === 'number') {
    // For now, treat all numbers as int32
    // In a more complete implementation, we'd need more type info
    visitor.visitInt(accessor);
  } else if (typeName === 'bigint') {
    visitor.visitLong(accessor);
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
