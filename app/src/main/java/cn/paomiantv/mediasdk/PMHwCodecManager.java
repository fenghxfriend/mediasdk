package cn.paomiantv.mediasdk;

import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;

import java.util.ArrayList;

public class PMHwCodecManager {
    private static class HwCodecManagerHolder {
        private static PMHwCodecManager instance = new PMHwCodecManager();
    }

    public static PMHwCodecManager instance() {
        return HwCodecManagerHolder.instance;
    }

    /**
     * Returns the first codec capable of encoding the specified MIME type, or null if no match was
     * found.
     */
    public String selectEncoder(MediaFormat format) {
        ArrayList<String> strings = new ArrayList<>();
        String mime = format.getString(MediaFormat.KEY_MIME);
        int num = MediaCodecList.getCodecCount();
        for (int i = 0; i < num; i++) {
            MediaCodecInfo info = MediaCodecList.getCodecInfoAt(i);
            if (!info.isEncoder()) {
                continue;
            }
            try {
                String[] types = info.getSupportedTypes();
                for (String type : types) {
                    if (type.equalsIgnoreCase(mime)) {
                        strings.add(info.getName());
                        break;
                    }
                }
            } catch (IllegalArgumentException e) {
                // type is not supported
                e.printStackTrace();
            }
        }
        if (strings.size() > 1) {
            for (int i = 0; i < strings.size(); i++) {
                boolean add = true;
                String name = strings.get(i);
                String[] a = name.split("\\.");
                for (String c : a) {
                    if (c.equalsIgnoreCase("google")) {
                        add = false;
                        break;
                    }
                }
                if (add) {
                    return name;
                }
            }
        }
        if (strings.size() > 0) {
            return strings.get(0);
        } else {
            return null;
        }
    }
}
