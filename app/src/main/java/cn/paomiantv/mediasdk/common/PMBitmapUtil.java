package cn.paomiantv.mediasdk.common;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.annotation.DrawableRes;
import android.support.annotation.NonNull;
import android.support.annotation.RawRes;

import java.io.ByteArrayOutputStream;

/**
 * Created by John on 2017/9/13.
 */

public class PMBitmapUtil {
    /**
     * Decode a drawable or a raw image resource.
     * @param res resources @see #ImgSdk.getAppResource().
     * @param resId resource id.
     * @param minSize minWidth the image must have.
     * @return the source resource image decoded with a minimum sample size.
     */
    public static Bitmap decodeResource(final Resources res, @DrawableRes @RawRes final int resId, final int minSize) {

        final BitmapFactory.Options opts = new BitmapFactory.Options();
        opts.inPreferredConfig = Bitmap.Config.ARGB_8888;
        opts.inJustDecodeBounds = true;
        BitmapFactory.decodeResource(res, resId, opts);

        final int size = Math.max(opts.outWidth, opts.outHeight);

        if (size > minSize && minSize > 0) {
            opts.inSampleSize = size / minSize;
        } else {
            opts.inSampleSize = 1;
        }

        limitMemoryUsage(opts);

        opts.inJustDecodeBounds = false;

        return BitmapFactory.decodeResource(res, resId, opts);
    }

    @NonNull
    public static float[] decodeSize(final Resources res, @DrawableRes @RawRes final int resId) {
        final BitmapFactory.Options opts = new BitmapFactory.Options();
        opts.inJustDecodeBounds = true;
        BitmapFactory.decodeResource(res, resId, opts);

        return new float[]{opts.outWidth, opts.outHeight};
    }

    @NonNull
    public static float[] decodeSize(final String filename) {
        final BitmapFactory.Options opts = new BitmapFactory.Options();
        opts.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(filename, opts);

        return new float[]{opts.outWidth, opts.outHeight};
    }

    public static Bitmap decodeFile(final String pathName, final int startInSampleSize) {
        final BitmapFactory.Options opts = new BitmapFactory.Options();

        opts.inJustDecodeBounds = true;
        opts.inPreferredConfig = Bitmap.Config.ARGB_8888;
        BitmapFactory.decodeFile(pathName, opts);
        limitMemoryUsage(opts);
        opts.inJustDecodeBounds = false;

        int inSampleSize = startInSampleSize;
        opts.inSampleSize = inSampleSize;
        opts.inDither = false;
        opts.inMutable = true;
        opts.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeFile(pathName, opts);
    }

    public static void limitMemoryUsage(@NonNull BitmapFactory.Options options) {

        float bufferScale = 2f;

        if (options.inSampleSize < 1) {
            options.inSampleSize = 1;
        }

        if (freeMemory() < ((options.outWidth * options.outHeight * 4) / (options.inSampleSize * options.inSampleSize)) * 1.5f) {
            System.gc();
            System.gc();
        }

        while (freeMemory() < ((options.outWidth * options.outHeight * 4) / (options.inSampleSize * options.inSampleSize)) * bufferScale) {
            options.inSampleSize += 1;
        }
    }

    public static long freeMemory() {
        return Runtime.getRuntime().maxMemory() - (Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory());
    }

    public static Bitmap argb2rgba(Bitmap img) {
        int width = img.getWidth();
        int height = img.getHeight();

        int[] pixelsIn = new int[height * width];
        int[] pixelsOut = new int[height * width];

        img.getPixels(pixelsIn, 0, width, 0, 0, width, height);
        if (img.isRecycled()) {
            img.recycle();
        }
        int pixel = 0;
        int count = width * height;

        while (count-- > 0) {
            int inVal = pixelsIn[pixel];

            //Get and set the pixel channel values from/to int  //TODO OPTIMIZE!
            int a = (int) ((inVal & 0xff000000) >> 24);
            int r = (int) ((inVal & 0x00ff0000) >> 16);
            int g = (int) ((inVal & 0x0000ff00) >> 8);
            int b = (int) (inVal & 0x000000ff);

            pixelsOut[pixel] = (int) (((r << 24) & 0xff000000) | ((g << 16) & 0x00ff0000) | ((b << 8) & 0x0000ff00) | (a & 0x000000ff));
            pixel++;
        }
        return Bitmap.createBitmap(pixelsOut, 0, width, width, height, Bitmap.Config.ARGB_8888);
    }

    public static Bitmap decodeByteArray(byte[] data) {
        if(data.length>=0){
            return BitmapFactory.decodeByteArray(data,0,data.length);
        }else{
            return null;
        }
    }

    public static byte[] Bitmap2Bytes(Bitmap bm) {
        if(bm!=null){
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            bm.compress(Bitmap.CompressFormat.PNG, 100, baos);
            return baos.toByteArray();
        }else{
            return null;
        }

    }
}
