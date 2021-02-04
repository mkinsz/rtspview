package org.mk.tview;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
//import android.net.wifi;
import android.net.wifi.WifiManager;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiConfiguration;
import android.net.DhcpInfo;

import java.util.List;

public class NotificationClient extends org.qtproject.qt5.android.bindings.QtActivity
{
    private static NotificationManager m_notificationManager;
    private static Notification.Builder m_builder;
    private static NotificationClient m_instance;
    private static List<ScanResult> wifiList;

    public static WifiInfo currentWifiInfo;

    public static DhcpInfo hostDhcpInfo;

    public NotificationClient()
    {
        m_instance = this;
    }

    public static String getName()
    {
        return "I Can Do It...";
    }

    public static void openWifi()
    {
        WifiManager man = (WifiManager) m_instance.getSystemService(Context.WIFI_SERVICE);
        if(!man.isWifiEnabled()){
            man.setWifiEnabled(true);
        }
    }

    public static void closeWifi()
    {
        WifiManager man = (WifiManager) m_instance.getSystemService(Context.WIFI_SERVICE);
        if(man.isWifiEnabled()){
            man.setWifiEnabled(false);
        }
    }

    public static void notify(String s)
    {
        if (m_notificationManager == null) {
            m_notificationManager = (NotificationManager)m_instance.getSystemService(Context.NOTIFICATION_SERVICE);
            m_builder = new Notification.Builder(m_instance);
            m_builder.setSmallIcon(R.drawable.icon);
            m_builder.setContentTitle("A message from Qt!");
        }

        m_builder.setContentText(s);
        m_notificationManager.notify(1, m_builder.build());
    }

    public static int networkState() {
        WifiManager man = (WifiManager)m_instance.getSystemService(Context.WIFI_SERVICE);
        return man.isWifiEnabled() ? 1 : 0;
    }

    public static void scanWifi()
    {
        WifiManager man = (WifiManager)m_instance.getSystemService(Context.WIFI_SERVICE);

        man.startScan();

        wifiList = man.getScanResults();
    }

    public static int wifiCount()
    {
        return wifiList.size();
    }

    public static String getWifiSSID(int index)
    {
        if(index >= 0 && index < wifiList.size()){

            return wifiList.get(index).SSID;
        } else{
            return "";
        }
    }

    public static String getWifiBSSID(int index)
    {
        if(index >= 0 && index < wifiList.size()){

            return wifiList.get(index).BSSID;
        } else{
            return "";
        }
    }

    public static int getWifiLevel(int index)
    {
        if(index >= 0 && index < wifiList.size()){

            return wifiList.get(index).level;
        } else{
            return 0;
        }
    }

    //加密方式判断
    public static String getWifiKeyType(int index)
    {
        if (wifiList.get(index).capabilities.contains("WEP")) {
            return "WEP";
        } else if (wifiList.get(index).capabilities.contains("PSK")) {
            return "PSK";
        } else if (wifiList.get(index).capabilities.contains("EAP")) {
            return "EAP";
        }
        return "无";
    }

    public static String getCurrentWifiSSID()
    {
        WifiManager man = (WifiManager)m_instance.getSystemService(Context.WIFI_SERVICE);

        currentWifiInfo = man.getConnectionInfo();

        return currentWifiInfo.getSSID();
    }

    public static String getHostIPAddress(){
        WifiManager man = (WifiManager)m_instance.getSystemService(Context.WIFI_SERVICE);
        hostDhcpInfo = man.getDhcpInfo();
        return m_instance.intToIp(hostDhcpInfo.serverAddress);
    }

    public static String getCurrentWifiIPAddress()
    {
        WifiManager man = (WifiManager)m_instance.getSystemService(Context.WIFI_SERVICE);

        currentWifiInfo = man.getConnectionInfo();

        return m_instance.intToIp(currentWifiInfo.getIpAddress());
    }

    //转换IP地址
    public static String intToIp(int i)
    {
        return (i & 0xFF) + "." + ((i >> 8) & 0xFF) + "." + ((i >> 16) & 0xFF)
        + "." + ((i >> 24) & 0xFF);
    }

    //生成一个网络配置
    public WifiConfiguration CreateWifiInfo(ScanResult scanresult, String password)
    {

        WifiConfiguration config = new WifiConfiguration();
        config.SSID = "\""+scanresult.SSID+"\"";     //这个地方一定要注意了。旁边的"是不能够省略的。密码的地方也一样。
        config.preSharedKey = "\""+password+"\"";    //该热点的密码
        config.hiddenSSID = true;

        config.status = WifiConfiguration.Status.ENABLED;
        config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
        config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
        config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
        config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
        config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
        config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        config.allowedProtocols.set(WifiConfiguration.Protocol.WPA);

        return config;
    }

    public static int connectToWifi(int index,String password)
    {
        WifiManager man = (WifiManager)m_instance.getSystemService(Context.WIFI_SERVICE);

        //AddNetWork(WifiConfiguration config);
        int id = man.addNetwork(m_instance.CreateWifiInfo(wifiList.get(index), password));

        if(id != -1){
            man.enableNetwork(id, false);
            man.saveConfiguration();
            return 1; //success
        }

        return 0; //falure
    }



}
