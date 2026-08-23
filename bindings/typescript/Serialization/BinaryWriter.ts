/**
 * Binary writer for serializing data to binary format
 * Writes in little-endian format to match C# implementation
 */
export class BinaryWriter {
  private static readonly DEFAULT_CAPACITY = 1024;

  private bytesWritten: number = 0;
  private capacity: number;
  private writeBuffer: Uint8Array;
  private dataView: DataView;

  constructor() {
    this.capacity = BinaryWriter.DEFAULT_CAPACITY;
    this.writeBuffer = new Uint8Array(this.capacity);
    this.dataView = new DataView(this.writeBuffer.buffer);
  }

  private ensureCapacity(additionalBytes: number): void {
    if (this.bytesWritten + additionalBytes > this.capacity) {
      this.capacity = Math.max(this.capacity * 2, this.bytesWritten + additionalBytes);
      const newBuffer = new Uint8Array(this.capacity);
      newBuffer.set(this.writeBuffer);
      this.writeBuffer = newBuffer;
      this.dataView = new DataView(this.writeBuffer.buffer);
    }
  }

  write(value: boolean): void;
  write(value: number): void;
  write(value: bigint): void;
  write(value: Uint8Array): void;
  write(value: boolean | number | bigint | Uint8Array): void {
    if (typeof value === 'boolean') {
      this.writeBoolean(value);
    } else if (typeof value === 'number') {
      this.writeByte(value);
    } else if (typeof value === 'bigint') {
      this.writeInt64(value);
    } else if (value instanceof Uint8Array) {
      this.writeBytes(value);
    }
  }

  writeBoolean(value: boolean): void {
    this.writeByte(value ? 1 : 0);
  }

  writeSByte(value: number): void {
    this.ensureCapacity(1);
    this.dataView.setInt8(this.bytesWritten, value);
    this.bytesWritten += 1;
  }

  writeByte(value: number): void {
    this.ensureCapacity(1);
    this.dataView.setUint8(this.bytesWritten, value);
    this.bytesWritten += 1;
  }

  writeInt16(value: number): void {
    this.ensureCapacity(2);
    this.dataView.setInt16(this.bytesWritten, value, true); // true = little endian
    this.bytesWritten += 2;
  }

  writeUInt16(value: number): void {
    this.ensureCapacity(2);
    this.dataView.setUint16(this.bytesWritten, value, true);
    this.bytesWritten += 2;
  }

  writeInt32(value: number): void {
    this.ensureCapacity(4);
    this.dataView.setInt32(this.bytesWritten, value, true);
    this.bytesWritten += 4;
  }

  writeUInt32(value: number): void {
    this.ensureCapacity(4);
    this.dataView.setUint32(this.bytesWritten, value, true);
    this.bytesWritten += 4;
  }

  writeInt64(value: bigint): void {
    this.ensureCapacity(8);
    this.dataView.setBigInt64(this.bytesWritten, value, true);
    this.bytesWritten += 8;
  }

  writeUInt64(value: bigint): void {
    this.ensureCapacity(8);
    this.dataView.setBigUint64(this.bytesWritten, value, true);
    this.bytesWritten += 8;
  }

  writeFloat(value: number): void {
    this.ensureCapacity(4);
    this.dataView.setFloat32(this.bytesWritten, value, true);
    this.bytesWritten += 4;
  }

  writeDouble(value: number): void {
    this.ensureCapacity(8);
    this.dataView.setFloat64(this.bytesWritten, value, true);
    this.bytesWritten += 8;
  }

  writeUTF8String(value: string): void {
    const encoder = new TextEncoder();
    const utf8Bytes = encoder.encode(value);

    const stringLength = utf8Bytes.length;
    this.writeInt32(stringLength);

    if (stringLength > 0) {
      this.writeBytes(utf8Bytes);
    }
  }

  writeBytes(value: Uint8Array): void {
    this.ensureCapacity(value.length);
    this.writeBuffer.set(value, this.bytesWritten);
    this.bytesWritten += value.length;
  }

  toArray(): Uint8Array {
    return this.writeBuffer.slice(0, this.bytesWritten);
  }
}
