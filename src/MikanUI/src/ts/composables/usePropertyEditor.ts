import {
  MikanVariant,
  MikanVariantType,
  MikanIntValue,
  MikanFloatValue,
  MikanBoolValue,
  MikanStringValue,
  MikanVector3fValue,
  MikanQuatfValue,
  MikanVector3f,
  MikanQuatf,
  CLASS_ID_MIKAN_BOOL_VALUE,
  CLASS_ID_MIKAN_INT_VALUE,
  CLASS_ID_MIKAN_FLOAT_VALUE,
  CLASS_ID_MIKAN_STRING_VALUE,
  CLASS_ID_MIKAN_VECTOR3F_VALUE,
  CLASS_ID_MIKAN_QUATF_VALUE
} from '@mikanxr/client'

/**
 * Composable for property editing utilities
 */
export function usePropertyEditor() {
  /**
   * Create a MikanVariant from a JavaScript value
   * Supports: boolean, number (int/float), string, Vector3, Quaternion
   */
  function createVariantFromValue(value: any): MikanVariant {
    const variant = new MikanVariant()

    if (typeof value === 'boolean') {
      variant.value_type = MikanVariantType.BOOL_TYPE
      const boolValue = new MikanBoolValue()
      boolValue.value = value
      variant.value_ptr.setInstance(boolValue, CLASS_ID_MIKAN_BOOL_VALUE, 'MikanBoolValue')
    } else if (typeof value === 'number') {
      if (Number.isInteger(value)) {
        variant.value_type = MikanVariantType.INT_TYPE
        const intValue = new MikanIntValue()
        intValue.value = value
        variant.value_ptr.setInstance(intValue, CLASS_ID_MIKAN_INT_VALUE, 'MikanIntValue')
      } else {
        variant.value_type = MikanVariantType.FLOAT_TYPE
        const floatValue = new MikanFloatValue()
        floatValue.value = value
        variant.value_ptr.setInstance(floatValue, CLASS_ID_MIKAN_FLOAT_VALUE, 'MikanFloatValue')
      }
    } else if (typeof value === 'string') {
      variant.value_type = MikanVariantType.MK_STRING_TYPE
      const stringValue = new MikanStringValue()
      stringValue.value = value
      variant.value_ptr.setInstance(stringValue, CLASS_ID_MIKAN_STRING_VALUE, 'MikanStringValue')
    } else if (typeof value === 'object' && value !== null) {
      // Check if it's a Vector3
      if ('x' in value && 'y' in value && 'z' in value && !('w' in value)) {
        variant.value_type = MikanVariantType.VECTOR3F_TYPE
        const vec3Value = new MikanVector3fValue()
        vec3Value.value = new MikanVector3f()
        vec3Value.value.x = value.x
        vec3Value.value.y = value.y
        vec3Value.value.z = value.z
        variant.value_ptr.setInstance(vec3Value, CLASS_ID_MIKAN_VECTOR3F_VALUE, 'MikanVector3fValue')
      }
      // Check if it's a Quaternion
      else if ('w' in value && 'x' in value && 'y' in value && 'z' in value) {
        variant.value_type = MikanVariantType.QUATERNIONF_TYPE
        const quatValue = new MikanQuatfValue()
        quatValue.value = new MikanQuatf()
        quatValue.value.w = value.w
        quatValue.value.x = value.x
        quatValue.value.y = value.y
        quatValue.value.z = value.z
        variant.value_ptr.setInstance(quatValue, CLASS_ID_MIKAN_QUATF_VALUE, 'MikanQuatfValue')
      }
    }

    return variant
  }

  return {
    createVariantFromValue
  }
}
