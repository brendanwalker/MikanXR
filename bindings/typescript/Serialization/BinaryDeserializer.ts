import {
  ValueAccessor,
  IVisitor,
  visitObject,
  visitValue,
  FieldMetadata,
  getSerializationMetadata
} from './SerializationUtils.js';
import { BinaryReader } from './BinaryReader.js';
import { PolymorphicObject, PolymorphicStruct } from '../PolymorphicObject.js';
import { TypeRegistry } from './JsonDeserializer.js';
import { EnumRegistry } from './EnumRegistry.js';

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

    // Get element type from metadata if available
    const fieldMetadata = (accessor as any).fieldMetadata;
    const elementTypeName = fieldMetadata?.type || 'number';

    // Deserialize each element of the array
    for (let arrayIndex = 0; arrayIndex < arraySize; arrayIndex++) {
      let element: any;

      // Check if it's a complex type with metadata
      const elementType = TypeRegistry.get(elementTypeName);
      if (elementType) {
        element = new elementType();
        visitObject(element, elementType, this);
      } else {
        // Primitive type - read based on type name
        element = this.readPrimitiveValue(elementTypeName);
      }

      list.push(element);
    }
  }

  private readPrimitiveValue(typeName: string): any {
    switch (typeName) {
      case 'boolean':
        return this.reader.readBoolean();
      case 'int8':
        return this.reader.readSByte();
      case 'uint8':
        return this.reader.readByte();
      case 'int16':
        return this.reader.readInt16();
      case 'uint16':
        return this.reader.readUInt16();
      case 'int32':
      case 'number':
        return this.reader.readInt32();
      case 'uint32':
        return this.reader.readUInt32();
      case 'int64':
      case 'bigint':
        return this.reader.readInt64();
      case 'uint64':
        return this.reader.readUInt64();
      case 'float':
        return this.reader.readSingle();
      case 'double':
        return this.reader.readDouble();
      case 'string':
        return this.reader.readUTF8String();
      default:
        return this.reader.readInt32();
    }
  }

  visitDictionary(accessor: ValueAccessor): void {
    // Get the record to be deserialized (should be a plain object/Record, not a Map)
    const record = accessor.getValueObject() as Record<any, any>;

    // Read the size of the record
    const arraySize = this.reader.readInt32();

    // Clear existing properties
    for (const key in record) {
      delete record[key];
    }

    // Get key and value types from metadata if available
    const fieldMetadata = (accessor as any).fieldMetadata;
    const keyTypeName = fieldMetadata?.keyType || 'number';
    const valueTypeName = fieldMetadata?.valueType || 'number';

    // Deserialize each pair
    for (let pairIndex = 0; pairIndex < arraySize; pairIndex++) {
      // Read key
      const key = this.readPrimitiveValue(keyTypeName);

      // Read value
      let value: any;
      const valueType = TypeRegistry.get(valueTypeName);
      if (valueType) {
        value = new valueType();
        visitObject(value, valueType, this);
      } else {
        value = this.readPrimitiveValue(valueTypeName);
      }

      record[key] = value;
    }
  }

  visitPolymorphicObject(accessor: ValueAccessor): void {
    // Get the PolymorphicObject container
    accessor.ensureValueAllocated();
    const polymorphicObject = accessor.getValueObject() as PolymorphicObject;
    const fieldName = accessor.valueName;

    // Read the runtime class info
    const elementRuntimeTypeName = this.reader.readUTF8String();
    const classId = this.reader.readInt64();

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
      polymorphicObject.setInstance(elementInstance, classId, elementRuntimeTypeName);
    }
  }

  visitEnum(accessor: ValueAccessor): void {
    const enumStringValue = this.reader.readUTF8String();
    // Try to parse as a numeric string first (e.g. "0")
    const numericValue = parseInt(enumStringValue, 10);
    if (!isNaN(numericValue)) {
      accessor.setValueObject(numericValue);
    } else {
      // Try to convert string enum name (e.g. "NONE") to integer via registry
      const fieldMetadata = (accessor as any).fieldMetadata;
      const enumTypeName = fieldMetadata?.type?.startsWith('enum:')
        ? fieldMetadata.type.substring(5)
        : null;
      if (enumTypeName) {
        const intValue = EnumRegistry.stringToInt(enumTypeName, enumStringValue);
        if (intValue !== null) {
          accessor.setValueObject(intValue);
          return;
        }
      }
      // Fall back to storing as string if registry lookup fails
      accessor.setValueObject(enumStringValue);
    }
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
    console.error('Failed to deserialize from binary:');
    console.error('Error:', e);
    if (e instanceof Error) {
      console.error('Stack:', e.stack);
    }
    return false;
  }
}
