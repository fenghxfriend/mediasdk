package cn.paomiantv.mediasdk.module;

public class PMModule {
    public long getNativeAddress() {
        return this.mNativeAddress;
    }

    // member accessed by native methods.
    protected long mNativeAddress;
}
