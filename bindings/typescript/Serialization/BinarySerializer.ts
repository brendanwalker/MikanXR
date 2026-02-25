import {
  ValueAccessor,
  IVisitor,
  visitObject,
  visitValue,
  FieldMetadata,
  getSerializationMetadata
} from './SerializationUtils';
import { BinaryWriter } from './BinaryWriter';
import { PolymorphicObject } from '../PolymorphicObject';
import { TypeRegistry } from './JsonDeserializer';

/**
 * Binary write visitor for serializing objects to binary format
 */
class BinaryWriteVisitor implements IVisitor {
  private writer: BinaryWriter;

  constructor(writer: BinaryWriter) {
    this.writer = writer;
  }

  visitClass(accessor: ValueAccessor): void {
    const objectValue = accessor.getValueObject();
    const objectType = accessor.valueType;

    // Write into the current container element
    visitObject(objectValue, objectType, this);
  }

  visitList(accessor: ValueAccessor): void {
    const list = accessor.getValueObject() as any[];

    // Write the size of the array
    const arraySize = list ? list.length : 0;
    this.writer.writeInt32(arraySize);

    // Get element type from metadata if available
    const fieldMetadata = (accessor as any).fieldMetadata;
    const elementTypeName = fieldMetadata?.type || 'number';

    // Serialize each element of the array
    if (list) {
      for (const element of list) {
        // Check if it's a complex type with metadata
        const elementType = TypeRegistry.get(elementTypeName);
        if (elementType) {
          visitObject(element, elementType, this);
        } else {
          // Primitive type - write based on type name
          this.writePrimitiveValue(element, elementTypeName);
        }
      }
    }
  }

  private writePrimitiveValue(value: any, typeName: string): void {
    switch (typeName) {
      case 'boolean':
        this.writer.writeBoolean(value);
        break;
      case 'int8':
        this.writer.writeSByte(value);
        break;
      case 'uint8':
        this.writer.writeByte(value);
        break;
      case 'int16':
        this.writer.writeInt16(value);
        break;
      case 'uint16':
        this.writer.writeUInt16(value);
        break;
      case 'int32':
      case 'number':
        this.writer.writeInt32(value);
        break;
      case 'uint32':
        this.writer.writeUInt32(value);
        break;
      case 'int64':
      case 'bigint':
        this.writer.writeInt64(value);
        break;
      case 'uint64':
        this.writer.writeUInt64(value);
        break;
      case 'float':
        this.writer.writeFloat(value);
        break;
      case 'double':
        this.writer.writeDouble(value);
        break;
      case 'string':
        this.writer.writeUTF8String(value);
        break;
      default:
        this.writer.writeInt32(value);
        break;
    }
  }

  visitDictionary(accessor: ValueAccessor): void {
    const map = accessor.getValueObject() as Map<any, any>;

    // Write the number of pairs in the map
    const arraySize = map ? map.size : 0;
    this.writer.writeInt32(arraySize);

    // Get key and value types from metadata if available
    const fieldMetadata = (accessor as any).fieldMetadata;
    const keyTypeName = fieldMetadata?.keyType || 'number';
    const valueTypeName = fieldMetadata?.valueType || 'number';

    // Serialize each key-value pair in the map
    if (map) {
      for (const [key, value] of map.entries()) {
        // Serialize the key
        this.writePrimitiveValue(key, keyTypeName);

        // Serialize the value
        const valueType = TypeRegistry.get(valueTypeName);
        if (valueType) {
          visitObject(value, valueType, this);
        } else {
          this.writePrimitiveValue(value, valueTypeName);
        }
      }
    }
  }

  visitPolymorphicObject(accessor: ValueAccessor): void {
    const polymorphicObject = accessor.getValueObject() as PolymorphicObject;
    const instance = polymorphicObject?.instance;
    const classId = polymorphicObject?.runtimeClassId || 0n;

    let className = '';
    let instanceType: any = null;

    if (instance && classId !== 0n) {
      instanceType = instance.constructor;
      className = instanceType.name;
    }

    this.writer.writeUTF8String(className);
    this.writer.writeInt64(classId);

    const isValidObject = instance !== null;
    this.writer.writeBoolean(isValidObject);

    // Only bother serializing the object if it's valid
    if (isValidObject && instance) {
      visitObject(instance, instanceType, this);
    }
  }

  visitEnum(accessor: ValueAccessor): void {
    const enumValue = accessor.getValueObject();
    // Convert enum to string name
    const enumStringValue = String(enumValue);
    this.writer.writeUTF8String(enumStringValue);
  }

  visitBool(accessor: ValueAccessor): void {
    this.writer.writeBoolean(accessor.getValue<boolean>());
  }

  visitByte(accessor: ValueAccessor): void {
    this.writer.writeSByte(accessor.getValue<number>());
  }

  visitUByte(accessor: ValueAccessor): void {
    this.writer.writeByte(accessor.getValue<number>());
  }

  visitShort(accessor: ValueAccessor): void {
    this.writer.writeInt16(accessor.getValue<number>());
  }

  visitUShort(accessor: ValueAccessor): void {
    this.writer.writeUInt16(accessor.getValue<number>());
  }

  visitInt(accessor: ValueAccessor): void {
    this.writer.writeInt32(accessor.getValue<number>());
  }

  visitUInt(accessor: ValueAccessor): void {
    this.writer.writeUInt32(accessor.getValue<number>());
  }

  visitLong(accessor: ValueAccessor): void {
    this.writer.writeInt64(accessor.getValue<bigint>());
  }

  visitULong(accessor: ValueAccessor): void {
    this.writer.writeUInt64(accessor.getValue<bigint>());
  }

  visitFloat(accessor: ValueAccessor): void {
    this.writer.writeFloat(accessor.getValue<number>());
  }

  visitDouble(accessor: ValueAccessor): void {
    this.writer.writeDouble(accessor.getValue<number>());
  }

  visitString(accessor: ValueAccessor): void {
    this.writer.writeUTF8String(accessor.getValue<string>());
  }
}

/**
 * Serialize an object to binary bytes
 */
export function serializeToBytes(instance: any, instanceType?: any): Uint8Array {
  const type = instanceType || instance?.constructor;
  const writer = new BinaryWriter();
  const visitor = new BinaryWriteVisitor(writer);

  visitObject(instance, type, visitor);

  return writer.toArray();
}
