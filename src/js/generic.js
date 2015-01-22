grpc_generic = function() {
  function FormatMessage(message) {
    var msglen = message.length;
    var out = new Uint8Array(5 + msglen);
    out[0] = 0;
    out[1] = (msglen >> 24) & 0xff;
    out[2] = (msglen >> 16) & 0xff;
    out[3] = (msglen >> 8) & 0xff;
    out[4] = msglen & 0xff;
    out.set(message, 5);
    return out;
  }

  function ParseMessage(response) {
    if (response.length < 5) return [null, response];
    if (response[0] != 0) return [null, response];
    var msglen = (response[0] << 24) | (response[1] << 16) | (response[2] << 8) | response[3];
    if (response.length < 5 + msglen) return [null, response];
    return [response.slice(5, 5 + msglen), response.slice(5 + msglen)];
  }

  function SetHeadersFromMetadata(req, metadata) {
    for (var key in metadata) {
      // TODO(ctiller): -bin headers
      req.setRequestHeader(key, metadata[key]);
    }
  }

  return {
    CreateGenericStub: function(host) {
      return {
        UnaryCall: function(method, metadata, request, response_cb) {
          var req = new XmlHttpRequest();
          req.open("POST", "https://" + host + "/" + method);
          SetHeadersFromMetadata(req, metadata);
          req.onreadystatechange = function() {
            if (req.readyState == req.DONE) {
              var response = ParseMessage(req.response);
              if (!response[0]) throw "Expected a grpc message to be returned";
              if (response[1] && response[1].length) throw "Trailing bytes after grpc message";
              response_cb(response[0]);
            }
          };
          req.responseType = 'arraybuffer';
          req.send(FormatMessage(request));
        },
      };
    }
  };
}();

