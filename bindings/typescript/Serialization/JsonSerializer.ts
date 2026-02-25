import {
  ValueAccessor,
  IVisitor,
  visitObject,
  visitValue,
  FieldMetadata,
  getSerializationMetadata
} from './SerializationUtils';
import { PolymorphicObject } from '../PolymorphicObject';

/**
 * JSON write visitor for serializing objects to JSON
 */
class JsonWriteVisitor implements IVisitor {
  private jsonToken: any;

  constructor(jsonToken: any = {}) {
    this.jsonToken = jsonToken;
  }

  getJsonToken(): any {
    return this.jsonToken;
  }

  visitClass(accessor: ValueAccessor): void {
    const objectValue = accessor.getValueObject();
    const jsonObject: any = {};
    const jsonVisitor = new JsonWriteVisitor(jsonObject);

    visitObject(objectValue, accessor.valueType, jsonVisitor);

    this.setJsonValueFromJsonToken(accessor.valueField, jsonObject);
  }

  visitList(accessor: ValueAccessor): void {
    const list = accessor.getValueObject() as any[];
    const jsonArray: any[] = [];

    if (list) {
      for (const element of list) {
        // Create accessor for array element
        const elementType = element?.constructor || Object;
        const elementAccessor = new ValueAccessor(element, elementType);

        // Serialize the element
        const elementVisitor = new JsonWriteVisitor(null);
        const metadata = getSerializationMetadata(element);

        if (metadata.length > 0) {
          visitObject(element, elementType, elementVisitor);
        } else {
          // Primitive or simple value
          elementVisitor.jsonToken = element;
        }

        jsonArray.push(elementVisitor.getJsonToken());
      }
    }

    this.setJsonValueFromJsonToken(accessor.valueField, jsonArray);
  }

  visitDictionary(accessor: ValueAccessor): void {
    const map = accessor.getValueObject() as Map<any, any>;
    const jsonPairArray: any[] = [];

    if (map) {
      for (const [key, value] of map.entries()) {
        const keyVisitor = new JsonWriteVisitor(null);
        keyVisitor.jsonToken = key;

        const valueVisitor = new JsonWriteVisitor(null);
        const valueMetadata = getSerializationMetadata(value);

        if (valueMetadata.length > 0) {
          visitObject(value, value.constructor, valueVisitor);
        } else {
          valueVisitor.jsonToken = value;
        }

        jsonPairArray.push({
          key: keyVisitor.getJsonToken(),
          value: valueVisitor.getJsonToken()
        });
      }
    }

    this.setJsonValueFromJsonToken(accessor.valueField, jsonPairArray);
  }

  visitPolymorphicObject(accessor: ValueAccessor): void {
    const polymorphicObject = accessor.getValueObject() as PolymorphicObject;
    const instance = polymorphicObject?.instance;
    const classId = polymorphicObject?.runtimeClassId || 0n;

    let className = '';
    const jsonRuntimeObject: any = {};

    if (classId !== 0n && instance) {
      className = instance.constructor.name;

      // Serialize the instance
      const jsonVisitor = new JsonWriteVisitor(jsonRuntimeObject);
      visitObject(instance, instance.constructor, jsonVisitor);
    }

    const jsonSerializableObject = {
      class_name: className,
      class_id: classId.toString(), // Convert bigint to string for JSON
      value: jsonRuntimeObject
    };

    this.setJsonValueFromJsonToken(accessor.valueField, jsonSerializableObject);
  }

  visitEnum(accessor: ValueAccessor): void {
    const enumValue = accessor.getValueObject();
    // Enums in TypeScript can be number or string
    this.setJsonValueFromJsonToken(accessor.valueField, enumValue);
  }

  visitBool(accessor: ValueAccessor): void {
    this.setJsonValueFromAccessor<boolean>(accessor);
  }

  visitByte(accessor: ValueAccessor): void {
    this.setJsonValueFromAccessor<number>(accessor);
  }

  visitUByte(accessor: ValueAccessor): void {
    this.setJsonValueFromAccessor<number>(accessor);
  }

  visitShort(accessor: ValueAccessor): void {
    this.setJsonValueFromAccessor<number>(accessor);
  }

  visitUShort(accessor: ValueAccessor): void {
    this.setJsonValueFromAccessor<number>(accessor);
  }

  visitInt(accessor: ValueAccessor): void {
    this.setJsonValueFromAccessor<number>(accessor);
  }

  visitUInt(accessor: ValueAccessor): void {
    this.setJsonValueFromAccessor<number>(accessor);
  }

  visitLong(accessor: ValueAccessor): void {
    const value = accessor.getValue<bigint>();
    // Convert bigint to string for JSON compatibility
    this.setJsonValueFromJsonToken(accessor.valueField, value.toString());
  }

  visitULong(accessor: ValueAccessor): void {
    const value = accessor.getValue<bigint>();
    this.setJsonValueFromJsonToken(accessor.valueField, value.toString());
  }

  visitFloat(accessor: ValueAccessor): void {
    this.setJsonValueFromAccessor<number>(accessor);
  }

  visitDouble(accessor: ValueAccessor): void {
    this.setJsonValueFromAccessor<number>(accessor);
  }

  visitString(accessor: ValueAccessor): void {
    const value = accessor.getValue<string>();
    this.setJsonValueFromJsonToken(accessor.valueField, value ?? '');
  }

  private setJsonValueFromAccessor<T>(accessor: ValueAccessor): void {
    const value = accessor.getValue<T>();
    this.setJsonValueFromJsonToken(accessor.valueField, value);
  }

  private setJsonValueFromJsonToken(sourceField: string | null, token: any): void {
    if (sourceField !== null) {
      this.jsonToken[sourceField] = token;
    } else {
      this.jsonToken = token;
    }
  }
}

/**
 * Serialize an object to JSON string
 */
export function serializeToJsonString(instance: any, instanceType?: any): string {
  const json = serializeToJson(instance, instanceType);
  return JSON.stringify(json);
}

/**
 * Serialize an object to JSON object
 */
export function serializeToJson(instance: any, instanceType?: any): any {
  const type = instanceType || instance?.constructor;
  const jsonObject: any = {};
  const visitor = new JsonWriteVisitor(jsonObject);

  visitObject(instance, type, visitor);

  return jsonObject;
}
