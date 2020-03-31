package com.example.sgnssid;

import java.util.List;

public interface OnDataListener {
    String getName();
    void onUDPPacket(final String respPkt);
}
