package org.mokee.settings.device;

import org.mokee.internal.util.FileUtils;

public class FreqTweaks {

	public static String CORE_BASE_PATH = "/sys/devices/system/cpu/";

	public static String[] getLittleAvailableFreq(){
		String result = FileUtils.readOneLine(CORE_BASE_PATH + "cpu0" + "/cpufreq/scaling_available_frequencies").trim();

		return result.split(" ");
	}

	public static String[] getBigAvailableFreq(){

		String result = FileUtils.readOneLine(CORE_BASE_PATH + "cpu4" + "/cpufreq/scaling_available_frequencies").trim();

		return result.split(" ");
	}

	public static String getMaxFreq(boolean isBigCore) {
		String path = CORE_BASE_PATH + (isBigCore ? "cpu4" : "cpu0") + "/cpufreq/scaling_max_freq";

		return FileUtils.readOneLine(path).trim();
	}

	public static String getMinFreq(boolean isBigCore) {
		String path = CORE_BASE_PATH + (isBigCore ? "cpu4" : "cpu0") + "/cpufreq/scaling_min_freq";

		return FileUtils.readOneLine(path).trim();
	}

	public static void setMaxFreq(boolean isBigCore,String freq){
		if (freq.equals("0")) return;
		String path = CORE_BASE_PATH + (isBigCore ? "cpu4" : "cpu0") + "/cpufreq/scaling_max_freq";
		FileUtils.writeLine(path,freq);
	}

	public static void setMinFreq(boolean isBigCore,String freq){
		if (freq.equals("0")) return;
		String path = CORE_BASE_PATH + (isBigCore ? "cpu4" : "cpu0") + "/cpufreq/scaling_min_freq";
		FileUtils.writeLine(path,freq);
	}
}