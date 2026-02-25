import {
  ValueAccessor,
  IVisitor,
  visitObject,
  visitValue,
  FieldMetadata,
  getSerializationMetadata
} from './SerializationUtils';
import { BinaryReader } from './BinaryReader';
import { PolymorphicObject, PolymorphicStruct } from '../PolymorphicObject';
import { TypeRegistry } from './JsonDeserializer';

/**
 * Binary read visitor for deserializing objects from binary format
 */
class BinaryReadVisitor implements IVisitor {
  private reader: BinaryReader;

  constructor(reader: BinaryReader) {
    this.reader = reader;
  }

  visitClass(accessor: ValueAccessor): void {
    accessor.ensureValueAllocated();
    const objectValue = accessor.getValueObject();
    const objectType = accessor.valueType;

    // Read into the current container element
    visitObject(objectValue, objectType, this);
  }

  visitList(accessor: ValueAccessor): void {
    // Get the list to be deserialized
    accessor.ensureValueAllocated();
    const list = accessor.getValueObject() as any[];

    // Read the size of the array
    const arraySize = this.reader.readInt32();

    // Clear the list
    list.length = 0;

    // Deserialize each element of the array
    for (let arrayIndex = 0; arrayIndex < arraySize; arrayIndex++) {
      // This is simplified - in a real implementation we'd need element type info
      // For now, just read as int32
      const element = this.reader.readInt32();
      list.push(element);
    }
  }

  visitDictionary(accessor: ValueAccessor): void {
    // Get the map to be deserialized
    accessor.ensureValueAllocated();
    const map = accessor.getValueObject() as Map<any, any>;

    // Read the size of the map
    const arraySize = this.reader.readInt32();

    // Clear the map
    map.clear();

    // Deserialize each pair
    for (let pairIndex = 0; pairIndex < arraySize; pairIndex++) {
      // Simplified - read as int32 keys and values
      const key = this.reader.readInt32();
      const value = this.reader.readInt32();
      map.set(key, value);
    }
  }

  visitPolymorphicObject(accessor: ValueAccessor): void {
    // Get the PolymorphicObject container
    accessor.ensureValueAllocated();
    const polymorphicObject = accessor.getValueObject() as PolymorphicObject;
    const fieldName = accessor.valueName;

    // Read the runtime class info
    const elementRuntimeTypeName = this.reader.readUTF8String();
    const classId = this.reader.readUInt64();

    const isValidObject = this.reader.readBoolean();
    if (isValidObject) {
      // Look up the runtime type
      const elementRuntimeType = TypeRegistry.get(elementRuntimeTypeName);
      if (!elementRuntimeType) {
        throw new Error(
          `BinaryDeserializer::visitPolymorphicObject() ` +
          `PolymorphicObject Accessor ${fieldName} ` +
          `used an unknown runtime class_name: ${elementRuntimeTypeName}`
        );
      }

      // Create instance
      const elementInstance = new elementRuntimeType();

      // Deserialize the instance
      visitObject(elementInstance, elementRuntimeType, this);

      // Set the instance on the PolymorphicObject
      polymorphicObject.setInstance(elementInstance, classId);
    }
  }

  visitEnum(accessor: ValueAccessor): void {
    const enumStringValue = this.reader.readUTF8String();
    // In TypeScript, we'd need the enum object to convert string back to value
    // For now, just store the string
    accessor.setValueObject(enumStringValue);
  }

  visitBool(accessor: ValueAccessor): void {
    const value = this.reader.readBoolean();
    accessor.setValue(value);
  }

  visitByte(accessor: ValueAccessor): void {
    const value = this.reader.readSByte();
    accessor.setValue(value);
  }

  visitUByte(accessor: ValueAccessor): void {
    const value = this.reader.readByte();
    accessor.setValue(value);
  }

  visitShort(accessor: ValueAccessor): void {
    const value = this.reader.readInt16();
    accessor.setValue(value);
  }

  visitUShort(accessor: ValueAccessor): void {
    const value = this.reader.readUInt16();
    accessor.setValue(value);
  }

  visitInt(accessor: ValueAccessor): void {
    const value = this.reader.readInt32();
    accessor.setValue(value);
  }

  visitUInt(accessor: ValueAccessor): void {
    const value = this.reader.readUInt32();
    accessor.setValue(value);
  }

  visitLong(accessor: ValueAccessor): void {
    const value = this.reader.readInt64();
    accessor.setValue(value);
  }

  visitULong(accessor: ValueAccessor): void {
    const value = this.reader.readUInt64();
    accessor.setValue(value);
  }

  visitFloat(accessor: ValueAccessor): void {
    const value = this.reader.readSingle();
    accessor.setValue(value);
  }

  visitDouble(accessor: ValueAccessor): void {
    const value = this.reader.readDouble();
    accessor.setValue(value);
  }

  visitString(accessor: ValueAccessor): void {
    const value = this.reader.readUTF8String();
    accessor.setValue(value);
  }
}

/**
 * Deserialize an object from binary bytes
 */
export function deserializeFromBytes(
  inBytes: Uint8Array,
  instance: any,
  instanceType?: any
): boolean {
  try {
    const type = instanceType || instance?.constructor;
    const binaryReader = new BinaryReader(inBytes);
    const visitor = new BinaryReadVisitor(binaryReader);

    visitObject(instance, type, visitor);

    return true;
  } catch (e) {
    console.error('Failed to deserialize from binary:', e);
    return false;
  }
}
