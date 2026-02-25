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

    // Serialize each element of the array
    if (list) {
      for (const element of list) {
        const elementType = element?.constructor || Object;
        const elementAccessor = new ValueAccessor(element, elementType);
        const metadata = getSerializationMetadata(element);

        if (metadata.length > 0) {
          visitObject(element, elementType, this);
        } else {
          // For primitives, write directly
          // This is simplified - in a real implementation we'd need type info
          if (typeof element === 'number') {
            this.writer.writeInt32(element);
          } else if (typeof element === 'string') {
            this.writer.writeUTF8String(element);
          } else if (typeof element === 'boolean') {
            this.writer.writeBoolean(element);
          } else if (typeof element === 'bigint') {
            this.writer.writeInt64(element);
          }
        }
      }
    }
  }

  visitDictionary(accessor: ValueAccessor): void {
    const map = accessor.getValueObject() as Map<any, any>;

    // Write the number of pairs in the map
    const arraySize = map ? map.size : 0;
    this.writer.writeInt32(arraySize);

    // Serialize each key-value pair in the map
    if (map) {
      for (const [key, value] of map.entries()) {
        // Serialize the key
        if (typeof key === 'number') {
          this.writer.writeInt32(key);
        } else if (typeof key === 'string') {
          this.writer.writeUTF8String(key);
        } else if (typeof key === 'bigint') {
          this.writer.writeInt64(key);
        }

        // Serialize the value
        const valueMetadata = getSerializationMetadata(value);
        if (valueMetadata.length > 0) {
          visitObject(value, value.constructor, this);
        } else {
          if (typeof value === 'number') {
            this.writer.writeInt32(value);
          } else if (typeof value === 'string') {
            this.writer.writeUTF8String(value);
          } else if (typeof value === 'boolean') {
            this.writer.writeBoolean(value);
          } else if (typeof value === 'bigint') {
            this.writer.writeInt64(value);
          }
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
