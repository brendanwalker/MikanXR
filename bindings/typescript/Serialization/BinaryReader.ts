/**
 * Binary reader for deserializing data from binary format
 * Reads in little-endian format to match C# implementation
 */
export class BinaryReader {
  private readBuffer: Uint8Array;
  private readBufferIndex: number = 0;
  private dataView: DataView;

  constructor(sourceBuffer: Uint8Array) {
    this.readBuffer = sourceBuffer;
    this.readBufferIndex = 0;
    this.dataView = new DataView(sourceBuffer.buffer, sourceBuffer.byteOffset, sourceBuffer.byteLength);
  }

  readBoolean(): boolean {
    return this.readByte() > 0;
  }

  readSByte(): number {
    const value = this.dataView.getInt8(this.readBufferIndex);
    this.readBufferIndex += 1;
    return value;
  }

  readByte(): number {
    const value = this.dataView.getUint8(this.readBufferIndex);
    this.readBufferIndex += 1;
    return value;
  }

  readInt16(): number {
    const value = this.dataView.getInt16(this.readBufferIndex, true); // true = little endian
    this.readBufferIndex += 2;
    return value;
  }

  readUInt16(): number {
    const value = this.dataView.getUint16(this.readBufferIndex, true);
    this.readBufferIndex += 2;
    return value;
  }

  readInt32(): number {
    const value = this.dataView.getInt32(this.readBufferIndex, true);
    this.readBufferIndex += 4;
    return value;
  }

  readUInt32(): number {
    const value = this.dataView.getUint32(this.readBufferIndex, true);
    this.readBufferIndex += 4;
    return value;
  }

  readInt64(): bigint {
    const value = this.dataView.getBigInt64(this.readBufferIndex, true);
    this.readBufferIndex += 8;
    return value;
  }

  readUInt64(): bigint {
    const value = this.dataView.getBigUint64(this.readBufferIndex, true);
    this.readBufferIndex += 8;
    return value;
  }

  readSingle(): number {
    const value = this.dataView.getFloat32(this.readBufferIndex, true);
    this.readBufferIndex += 4;
    return value;
  }

  readDouble(): number {
    const value = this.dataView.getFloat64(this.readBufferIndex, true);
    this.readBufferIndex += 8;
    return value;
  }

  readUTF8String(): string {
    const byteCount = this.readInt32();
    let utf8String = '';

    if (byteCount > 0) {
      const decoder = new TextDecoder('utf-8');
      const bytes = this.readBuffer.slice(this.readBufferIndex, this.readBufferIndex + byteCount);
      utf8String = decoder.decode(bytes);
      this.readBufferIndex += byteCount;
    }

    return utf8String;
  }
}
