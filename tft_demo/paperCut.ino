// papercut.ino

const char* pc_host = "https://print.rose-hulman.edu:9192";
const char* pc_user = "panh3";
const char* pc_pass = "Tuoma060831!";
const char* pc_printer = "print%5CD116"; 


bool papercutLogin(WiFiClientSecure &client, String &authCookie) {
  HTTPClient http;
  String url = String(pc_host) + "/rpc/api/rest/internal/mobilerelease/api/" + pc_user + "/log-in";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("X-Requested-With", "XMLHttpRequest");
  http.addHeader("Origin", pc_host);
  http.addHeader("Referer", String(pc_host) + "/mobile/release?");

  String b64pass = base64::encode(pc_pass);
  String body = "username=" + String(pc_user) + "&password=" + b64pass;

  int code = http.POST(body);
  Serial.print("Login: ");
  Serial.println(code);

  if (code == 200) {
    String resp = http.getString();
    Serial.println(resp);
    // 提取 authCookie
    int idx = resp.indexOf("\"authCookie\":\"");
    if (idx > 0) {
      idx += 14;
      int end = resp.indexOf("\"", idx);
      authCookie = resp.substring(idx, end);
    }
    http.end();
    return true;
  }
  http.end();
  return false;
}

String getHeldJobs(WiFiClientSecure &client) {
  HTTPClient http;
  String url = String(pc_host) + "/rpc/api/rest/internal/mobilerelease/api/held-jobs/?username=" + pc_user + "&printerName=" + pc_printer;
  http.begin(client, url);
  http.addHeader("X-Requested-With", "XMLHttpRequest");
  http.addHeader("Origin", pc_host);
  http.addHeader("Referer", String(pc_host) + "/mobile/release?");

  int code = http.GET();
  Serial.print("Jobs: ");
  Serial.println(code);

  String jobId = "";
  if (code == 200) {
    String resp = http.getString();
    Serial.println(resp);
    // 提取第一个 job id
    int idx = resp.indexOf("\"id\":\"");
    if (idx > 0) {
      idx += 6;
      int end = resp.indexOf("\"", idx);
      jobId = resp.substring(idx, end);
    }
  }
  http.end();
  return jobId;
}

bool releaseJob(WiFiClientSecure &client, String jobId) {
  HTTPClient http;
  String url = String(pc_host) + "/rpc/api/rest/internal/mobilerelease/api/held-jobs/release?username=" + pc_user;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("X-Requested-With", "XMLHttpRequest");
  http.addHeader("Origin", pc_host);
  http.addHeader("Referer", String(pc_host) + "/mobile/release?");

  String body = "printerName=" + String(pc_printer) + "&jobIds%5B%5D=" + jobId;

  int code = http.POST(body);
  Serial.print("Release: ");
  Serial.println(code);
  if (code == 200) {
    Serial.println(http.getString());
    http.end();
    return true;
  }
  http.end();
  return false;
}

void papercutReleaseAll() {
  WiFiClientSecure client;
  client.setInsecure();
  String authCookie = "";

  Serial.println("=== PaperCut Release ===");

  if (!papercutLogin(client, authCookie)) {
    Serial.println("Login failed");
    return;
  }

  String jobId = getHeldJobs(client);
  if (jobId.length() == 0) {
    Serial.println("No jobs to release");
    return;
  }

  Serial.println("Releasing: " + jobId);
  releaseJob(client, jobId);
  Serial.println("=== Done ===");
}