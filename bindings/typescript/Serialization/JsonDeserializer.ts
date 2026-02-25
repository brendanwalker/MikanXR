import {
  ValueAccessor,
  IVisitor,
  visitObject,
  visitValue,
  FieldMetadata,
  getSerializationMetadata
} from './SerializationUtils';
import { PolymorphicObject, PolymorphicStruct } from '../PolymorphicObject';

/**
 * Type registry for runtime type lookup
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

/**
 * JSON read visitor for deserializing objects from JSON
 */
class JsonReadVisitor implements IVisitor {
  private jsonToken: any;

  constructor(jsonToken: any) {
    this.jsonToken = jsonToken;
  }

  getJsonToken(): any {
    return this.jsonToken;
  }

  visitClass(accessor: ValueAccessor): void {
    const jsonToken = this.getJsonTokenFromAccessor(accessor);

    if (typeof jsonToken === 'object' && jsonToken !== null) {
      accessor.ensureValueAllocated();
      const fieldInstance = accessor.getValueObject();

      const jsonVisitor = new JsonReadVisitor(jsonToken);
      visitObject(fieldInstance, accessor.valueType, jsonVisitor);
    } else {
      throw new Error(
        `JsonReadVisitor::visitClass() Field ${accessor.valueName} missing corresponding json object`
      );
    }
  }

  visitList(accessor: ValueAccessor): void {
    const jsonToken = this.getJsonTokenFromAccessor(accessor);

    if (Array.isArray(jsonToken)) {
      accessor.ensureValueAllocated();
      const list = accessor.getValueObject() as any[];

      // Clear the list
      list.length = 0;

      for (const jsonElement of jsonToken) {
        // Determine element type - this is simplified, in real implementation
        // we'd need more type information
        let element: any;

        if (typeof jsonElement === 'object' && jsonElement !== null) {
          // For complex types, we need the metadata
          // For now, create a plain object
          element = jsonElement;
        } else {
          element = jsonElement;
        }

        list.push(element);
      }
    } else {
      throw new Error(
        `JsonReadVisitor::visitList() Field ${accessor.valueName} missing corresponding json array`
      );
    }
  }

  visitDictionary(accessor: ValueAccessor): void {
    const jsonToken = this.getJsonTokenFromAccessor(accessor);

    if (Array.isArray(jsonToken)) {
      accessor.ensureValueAllocated();
      const map = accessor.getValueObject() as Map<any, any>;

      map.clear();

      for (const jsonPair of jsonToken) {
        const key = jsonPair.key;
        const value = jsonPair.value;

        map.set(key, value);
      }
    } else {
      throw new Error(
        `JsonReadVisitor::visitDictionary() Field ${accessor.valueName} missing corresponding json array`
      );
    }
  }

  visitPolymorphicObject(accessor: ValueAccessor): void {
    const jsonToken = this.getJsonTokenFromAccessor(accessor);

    accessor.ensureValueAllocated();
    const fieldInstance = accessor.getValueObject() as PolymorphicObject;

    const elementRuntimeTypeName = jsonToken.class_name || '';
    if (elementRuntimeTypeName === '') {
      return;
    }

    // Look up the runtime type
    const elementRuntimeType = TypeRegistry.get(elementRuntimeTypeName);
    if (!elementRuntimeType) {
      throw new Error(
        `JsonReadVisitor::visitPolymorphicObject() ` +
        `PolymorphicObject Accessor ${accessor.valueName} ` +
        `used an unknown runtime class_name: ${elementRuntimeTypeName}`
      );
    }

    // Create instance
    const elementInstance = new elementRuntimeType();

    // Deserialize the element
    const elementJson = jsonToken.value;
    const elementVisitor = new JsonReadVisitor(elementJson);
    visitObject(elementInstance, elementRuntimeType, elementVisitor);

    // Set the instance on the PolymorphicObject
    const classId = BigInt(jsonToken.class_id || '0');
    fieldInstance.setInstance(elementInstance, classId);
  }

  visitEnum(accessor: ValueAccessor): void {
    const jsonToken = this.getJsonTokenFromAccessor(accessor);

    if (typeof jsonToken === 'number') {
      accessor.setValueObject(jsonToken);
    } else if (typeof jsonToken === 'string') {
      // Parse string enum value
      // In TypeScript, we'd need the enum object to convert
      accessor.setValueObject(jsonToken);
    } else {
      throw new Error(
        `JsonReadVisitor::visitEnum() Enum Accessor ${accessor.valueName} ` +
        `was not a number or string json value`
      );
    }
  }

  visitBool(accessor: ValueAccessor): void {
    const jsonToken = this.getJsonTokenFromAccessor(accessor);

    if (typeof jsonToken === 'boolean') {
      accessor.setValue(jsonToken);
    } else {
      throw new Error(
        `JsonReadVisitor::visitBool() Bool Accessor ${accessor.valueName} ` +
        `was not a boolean json value`
      );
    }
  }

  visitByte(accessor: ValueAccessor): void {
    this.visitNumeric(accessor, 'Byte');
  }

  visitUByte(accessor: ValueAccessor): void {
    this.visitNumeric(accessor, 'UByte');
  }

  visitShort(accessor: ValueAccessor): void {
    this.visitNumeric(accessor, 'Short');
  }

  visitUShort(accessor: ValueAccessor): void {
    this.visitNumeric(accessor, 'UShort');
  }

  visitInt(accessor: ValueAccessor): void {
    this.visitNumeric(accessor, 'Int');
  }

  visitUInt(accessor: ValueAccessor): void {
    this.visitNumeric(accessor, 'UInt');
  }

  visitLong(accessor: ValueAccessor): void {
    const jsonToken = this.getJsonTokenFromAccessor(accessor);

    if (typeof jsonToken === 'number' || typeof jsonToken === 'string') {
      const value = BigInt(jsonToken);
      accessor.setValue(value);
    } else {
      throw new Error(
        `JsonReadVisitor::visitLong() Long Accessor ${accessor.valueName} ` +
        `was not a number or string json value`
      );
    }
  }

  visitULong(accessor: ValueAccessor): void {
    const jsonToken = this.getJsonTokenFromAccessor(accessor);

    if (typeof jsonToken === 'number' || typeof jsonToken === 'string') {
      const value = BigInt(jsonToken);
      accessor.setValue(value);
    } else {
      throw new Error(
        `JsonReadVisitor::visitULong() ULong Accessor ${accessor.valueName} ` +
        `was not a number or string json value`
      );
    }
  }

  visitFloat(accessor: ValueAccessor): void {
    this.visitNumeric(accessor, 'Float');
  }

  visitDouble(accessor: ValueAccessor): void {
    this.visitNumeric(accessor, 'Double');
  }

  visitString(accessor: ValueAccessor): void {
    const jsonToken = this.getJsonTokenFromAccessor(accessor);

    if (typeof jsonToken === 'string') {
      accessor.setValue(jsonToken);
    } else {
      throw new Error(
        `JsonReadVisitor::visitString() String Accessor ${accessor.valueName} ` +
        `was not a string json value`
      );
    }
  }

  private visitNumeric(accessor: ValueAccessor, typeName: string): void {
    const jsonToken = this.getJsonTokenFromAccessor(accessor);

    if (typeof jsonToken === 'number') {
      accessor.setValue(jsonToken);
    } else {
      throw new Error(
        `JsonReadVisitor::visit${typeName}() ${typeName} Accessor ${accessor.valueName} ` +
        `was not a number json value`
      );
    }
  }

  private getJsonTokenFromAccessor(accessor: ValueAccessor): any {
    const field = accessor.valueField;
    if (field !== null) {
      if (this.jsonToken.hasOwnProperty(field)) {
        return this.jsonToken[field];
      }
      throw new Error(`Field ${field} not found in json`);
    } else {
      return this.jsonToken;
    }
  }
}

/**
 * Deserialize an object from JSON string
 */
export function deserializeFromJsonString(
  jsonString: string,
  instance: any,
  instanceType?: any
): boolean {
  try {
    const jsonObject = JSON.parse(jsonString);
    return deserializeFromJson(jsonObject, instance, instanceType);
  } catch (e) {
    console.error('Failed to deserialize from JSON string:', e);
    return false;
  }
}

/**
 * Deserialize an object from JSON object
 */
export function deserializeFromJson(
  jsonObject: any,
  instance: any,
  instanceType?: any
): boolean {
  try {
    const type = instanceType || instance?.constructor;
    const visitor = new JsonReadVisitor(jsonObject);

    visitObject(instance, type, visitor);

    return true;
  } catch (e) {
    console.error('Failed to deserialize from JSON:', e);
    return false;
  }
}
