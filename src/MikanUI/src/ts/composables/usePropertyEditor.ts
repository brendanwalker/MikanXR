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
  MikanQuatf
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
      variant.value_ptr.setInstance(boolValue)
    } else if (typeof value === 'number') {
      if (Number.isInteger(value)) {
        variant.value_type = MikanVariantType.INT_TYPE
        const intValue = new MikanIntValue()
        intValue.value = value
        variant.value_ptr.setInstance(intValue)
      } else {
        variant.value_type = MikanVariantType.FLOAT_TYPE
        const floatValue = new MikanFloatValue()
        floatValue.value = value
        variant.value_ptr.setInstance(floatValue)
      }
    } else if (typeof value === 'string') {
      variant.value_type = MikanVariantType.MK_STRING_TYPE
      const stringValue = new MikanStringValue()
      stringValue.value = value
      variant.value_ptr.setInstance(stringValue)
    } else if (typeof value === 'object' && value !== null) {
      // Check if it's a Vector3
      if ('x' in value && 'y' in value && 'z' in value && !('w' in value)) {
        variant.value_type = MikanVariantType.VECTOR3F_TYPE
        const vec3Value = new MikanVector3fValue()
        vec3Value.value = new MikanVector3f()
        vec3Value.value.x = value.x
        vec3Value.value.y = value.y
        vec3Value.value.z = value.z
        variant.value_ptr.setInstance(vec3Value)
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
        variant.value_ptr.setInstance(quatValue)
      }
    }

    return variant
  }

  return {
    createVariantFromValue
  }
}
