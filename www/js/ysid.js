var ysid = {
  items: {}
};

YUI().use("*", function(Y) { ysid.Y = Y; });

function ysid_log() {
var args = arguments;
  var Y = ysid.Y;
  for (var i in args) {
    Y.log(Y.dump(args[i]));
  }
}

function ysid_info(mesg, type, time) {
  var Y = ysid.Y;
  var info = Y.one("#info");
  if (!ysid.info_overlay) {
    var overlay = new Y.Overlay(
      {
        srcNode: "#info",
        visible: false
      });
    overlay.render();
    ysid.info_overlay = overlay;
  }
  type = type || "info";
  var dft_time = 2;
  if (type == "error") dft_time = 4;
  info.set("innerHTML", "<div></div>");
  time = time || dft_time;
  var d = info.one("div");
  if (type == "error") d.setStyle("color", "red");
  d.set("innerText", mesg);
  var anim = new Y.Anim(
    { node: d,
      to: { opacity: 0},
      duration: time
    });
  anim.on("end", function() { ysid.info_overlay.hide(); });
  ysid.info_overlay.set("align", {points: [Y.WidgetPositionAlign.TR, Y.WidgetPositionAlign.TR]});
  ysid.info_overlay.show();
  anim.run();
}

function ysid_error(mesg) {
  ysid_info(mesg, "error", 4);
}

ZeroClipboard.setMoviePath("/ZeroClipboard10.swf");
function ysid_copy(text, on_success, on_failure) {
  if (window.clipboardData && window.clipboardData.setData) { // ie
    try {
      window.clipboardData.setData("Text", text);
      if (on_success) on_success();
    } catch (x) {
      ysid_info("ie with window.clipboardData disabled ...");
    }
  }
  var Y = ysid.Y;
  Y.io("/clipboard/copy/", {
         method: "POST",
         data: {text: text},
         on: {
           start: function(id, args) {
             ysid_info("coping ...");
           },
           success: function(id, o, args) {
             if (o.responseText == "OK") {
               if (on_success) on_success();
               ysid_info("copied");
             } else {
               if (on_failure) on_failure();
               ysid_error("coping failed: " + o.responseText);
             }
           },
           failure: function(id, o, args) {
             if (on_failure) on_failure();
             ysid_error("coping failed: " + o.responseText);
           }
         }
       });
}

function YsidItem(config) {
  var Y = ysid.Y;
  if (typeof config == "string") {
    var a = config.split('\n');
    config = { };
    for (var i in a) {
      var f = a[i].split(':', 1)[0];
      var v = a[i].slice(f.length + 1);
      config[f] = v;
    }
  }
  config = config || {};
  this.uuid = config["uuid"] || "";
  this.username = config["username"] || "";
  this.password = config["password"] || "";
  this.url = config["url"] || "";
  this.tags = config["tags"] || "";
  this.notes = config["notes"] || "";
}

YsidItem.prototype.to_map = function() {
  return {
    uuid: this.uuid,
    username: this.username,
    password: this.password,
    url: this.url,
    tags: this.tags,
    notes: this.notes
  };  
};

YsidItem.prototype.from_form = function(edit) {
  this.uuid = edit.one("input[name=uuid]").get("value") || "";
  this.username = edit.one("input[name=username]").get("value") || "";
  this.password = edit.one("input[name=password]").get("value") || "",
  this.url =  edit.one("input[name=url]").get("value") || "";
  this.tags = edit.one("input[name=tags]").get("value") || "";
  this.notes =  edit.one("input[name=notes]").get("value") || "";
};

YsidItem.prototype.to_form = function(edit) {
  edit.one("input[name=uuid]").set("value", this.uuid || "");
  edit.one("input[name=username]").set("value", this.username || "");
  edit.one("input[name=url]").set("value", this.url || "");
  edit.one("input[name=tags]").set("value", this.tags || "");
  edit.one("input[name=notes]").set("value", this.notes || "");
};

YsidItem.prototype.to_html = function() {
var s = "<tr id=\"item" + this.uuid + "\" class='item'>"
    + "<td class='item-username'>" + this.username + "</td>"
    + "<td class='item-url'>";
  if (this.url.slice(0, 4) == "http") {
    s += "<a href='" + this.url + "'>" + this.url + "</a>"
  } else {
    s += this.url;
  }
  s += "</td>"
    + "<td class='item-tags'>" + this.tags + "</td>"
    + "<td class='item-notes'>" + this.notes + "</td>"
    + "</tr>";
  return s;
};

function YsidItemFilter(filter) {
  if (!filter) return;
  if (filter && filter[0] == '-') {
    this.reverse = true;
    filter = filter.slice(1);
  }
  var field = filter.split(':', 1)[0], value;
  if (field == filter) {
    field = "";
    value = filter;
  } else {
    value = filter.slice(field.length + 1);
  }
  function one_of(e, a) {
    for (var i in a) {
      if (a[i] == e) return true;
    }
    return false;
  }
  this.value = value;
  if (one_of(field, ["u", "user", "username"])) this.field = "username";
  if (one_of(field, ["l", "url"])) this.field = "url";
  if (one_of(field, ["n", "note", "notes"])) this.field = "notes";
  if (one_of(field, ["t", "tag", "tags"])) {
    this.field = "tags";
    this.value = value.split(',');
  }
  if (one_of(field, ["User", "Username"])) {
    this.field = "username";
    this.prefix = true;
  }
  if (one_of(field, ["U", "USER", "USERNAME"])) {
    this.field = "username";
    this.exact = true;
  }
  if (!this.field) this.value = filter;
}

YsidItemFilter.prototype.do_match = function(item) {
  var Y = ysid.Y;
  if (this.field) {
    if (this.field == "tags") {
      var tags = new Y.Array(item.tags.split(','));
      for (var i in this.value) {
        if (tags.indexOf(this.value[i]) == -1) return false;
      }
      return true;      
    }
    if (this.exact) return item[this.field] == this.value;
    var pos = item[this.field].search(this.value);
    if (this.prefix) return pos == 0;
    return pos != -1;
  }
  return item.username.search(this.value) != -1
    || item.url.search(this.value) != -1
    || item.notes.search(this.value) != -1
    || item.tags.search(this.value) != -1;
};

YsidItemFilter.prototype.match = function(item) {
  var r = this.do_match(item);
  if (this.reverse) return !r;
  return r;
};

YsidItem.prototype.match = function(filter) {
  if (!filter) return true;
  if (typeof filter == "string") filter = new YsidItemFilter(filter);
  return filter.match(this);
};

function ysid_menu_item_show_password() {
  var Y = ysid.Y;
  var pn = Y.one("#password_show");
  var uuid = ysid_get_current_item_uuid();
  if (uuid) {
    ysid_get_item_password(
      uuid, function (uuid, password) {
        pn.set("value", password);
      });
  }
  if (ysid.menu_overlay) ysid.menu_overlay.hide();
}

function ysid_menu_item_edit(field) {
  field = field || "username";
  var Y = ysid.Y;
  var uuid = ysid_get_current_item_uuid();
  if (uuid) {
    var item = ysid.items[uuid];
    ysid_edit_form(item, field);    
  }
  if (ysid.menu_overlay) ysid.menu_overlay.hide();
}

function ysid_menu_item_erase() {
var Y = ysid.Y;
  var uuid = ysid_get_current_item_uuid();
  if (uuid) {
    var item = ysid.items[uuid];
    var ans = window.confirm("Are you sure to delete " + item.username + "@" + item.url + "?");
    if (!ans) return;
    Y.io("/erase/" + uuid + "/", {
           method : "POST",
           on: {
             start: function(id, args) {
               ysid_info("deleting ...");
             },
             success: function(id, o, args) {
               if (o.responseText == "OK") {
                 ysid_info("deleted");
                 delete ysid.items[uuid];
                 ysid_list();
               } else {
                 ysid_error('delete failed: ' + o.responseText);
               }
             },
             failure: function(id, o, args) {
               ysid_error('delete failed: ' + o.responseText);
             }
           }
         });
  }
  if (ysid.menu_overlay) ysid.menu_overlay.hide();
}

YsidItem.prototype.is_empty = function() {
  return !this.username && !this.url && !this.tags && !this.notes;
};

function ysid_login_form() {
  var Y = ysid.Y;
  function from_storage(login) {
    try {
      var a = ["path", "dpri", "dpub", "spri", "spub"];
      login = Y.one(login);
      for (var i in a) {
        var k = a[i];
        login.one("[name=" + k + "]").set("value", Y.StorageLite.getItem(k) || "");      
      }
    } catch (x) {
      ysid_error("from_storage failed with " + x);
    }
  }
  function to_storage(login) {
    try {
      var a = ["path", "dpri", "dpub", "spri", "spub"];
      login = Y.one(login);
      for (var i in a) {
        var k = a[i];
        Y.StorageLite.setItem(k, login.one("[name=" + k + "]").get("value")); 
      }
    } catch (x) {
      ysid_error("to_storage failed with " + x);
    }
  }
  function show_login() {
    ysid.login_overlay.set("centered", true);
    from_storage("#login");
    ysid.login_overlay.show();
    Y.one("#login").one("input[name=path]").focus();
  }
  if (ysid.login_overlay) {
    show_login();
    return;
  }
  var login = Y.one("#login");
  var overlay = new Y.Overlay(
    {
      srcNode: "#login",
      visible: false,
      centered: true
    });
  ysid.login_overlay = overlay;
  overlay.render();
  show_login();
  var cancel = login.one("input[type=submit][value=cancel]");
  cancel.on("click", function (e) {
              e.preventDefault();
              overlay.hide();
            });
  var submit = login.one("input[type=submit]");
  submit.on("click", function (e) {
              e.preventDefault();
              var request = Y.io(
                "/login/",
                {
                  method: "POST",
                  form: { id: "login_form", useDisabled: false },
                  on: {
                    start: function(id, args) {
                      ysid_info("login ...");
                      login.one("input[type=password]").set("value", "");
                      to_storage("#login");
                    },
                    success: function(id, o, args) {
                      ysid_info("logined");
                      overlay.hide();
                      ysid_get_items(ysid_show_items);
                    },
                    failure: function(id, o, args) {
                      ysid_error("login failed: " + o.responseText);
                    }
                  }
                }
              );
            });
}

function ysid_edit_form(item, field) {
  var Y = ysid.Y;
  item = new YsidItem(item || {});
  field = field || "username";
  function show_edit() {
    ysid.edit_overlay.set("centered", true);
    ysid.edit_overlay.show();
    item.to_form(ysid.edit_overlay.edit);
    ysid.edit_overlay.edit.one("[name=password]").set("value", "");
    ysid.edit_overlay.edit.one("[name=" + field + "]").focus();
  }
  if (ysid.edit_overlay) {
    show_edit();
    return;
  }
  var edit = Y.one("#edit");
  var overlay = new Y.Overlay(
    {
      srcNode: "#edit",
      visible: false,
      centered: true
    });
  ysid.edit_overlay = overlay;
  ysid.edit_overlay.edit = edit;
  overlay.render();
  show_edit();
  var cancel = edit.one("input[type=submit][value=cancel]");
  cancel.on("click", function (e) {
              e.preventDefault();
              overlay.hide();
            });
  var submit = edit.one("input[type=submit][value=submit]");
  submit.on("click", function (e) {
              e.preventDefault();
              var edit = ysid.edit_overlay.edit;
              ysid_random_it(edit.one("[name=password]"), "");
              ysid_random_it(edit.one("[name=username]"), "");
              var item = new YsidItem();
              item.from_form(edit);
              if (item.is_empty()) {
                ysid_error("empty item is not submited");
                ysid.edit_overlay.hide();
                return;
              }
              var request = Y.io(
                "/put/",
                {
                  method: "POST",
                  form: { id: "edit_form", useDisabled: false },
                  on: {
                    start: function(id, o, args) {
                      ysid_info("edit ...");
                    },
                    success: function(id, o, args) {
                      ysid_info("edited");
                      overlay.hide();
                      item.password = "";
                      item.uuid = o.responseText;
                      ysid.items[item.uuid] = item;
                      ysid_list();
                    },
                    failure: function(id, o, args) {
                      ysid_error("edit failed: " + o.responseText);
                    }
                  }
                }
              );
            });
}

function ysid_logout() {
  var Y = ysid.Y;
  Y.io("/logout/", {
         method: "POST",
         on : {
           start: function(id, args) {
             ysid_info("logout ...");
           },
           success: function(id, o, args) {
             ysid_info("logout");
           },
           failure: function(id, o, args) {
             ysid_error("logout failed, try to kill the server");
           }
         }
       });
  ysid.items = { };
  ysid_show_items();
}

function ysid_get_item(uuid, on_get, on_fail) {
  var Y = ysid.Y;
  Y.io("/get/" + uuid + "/",
       {
         on: {
           start: function(id, args) {
             ysid_info("getting " + uuid + " ...");
           },
           success: function(id, o, args) {
             ysid_info("got " + uuid);
             var item = new YsidItem(o.responseText);
             ysid.items[uuid] = item;
             on_get(item);
           },
           failure: function(id, o, args) {
             ysid_error("getting " + uuid + " failed: " + o.responseText);
             if (on_fail) on_fail(uuid);
           }
         }
       });
}

function ysid_get_item_field(uuid, field, on_get, on_fail) {
  var Y = ysid.Y;
  Y.io("/get/" + uuid + "/" + field + "/",
       {
         on: {
           start: function(id, args) {
             ysid_info("getting " + uuid + "/" + field + " ...");
           },
           success: function(id, o, args) {
             ysid_info("got " + uuid + "/" + field);
             on_get(uuid, field, o.responseText);
           },
           failure: function(id, o, args) {
             ysid_error("getting " + uuid + "/" + field + " failed: " + o.responseText);
             if (on_fail) on_fail(uuid, field);
           }
         }
       }
      );
}

function ysid_get_item_password(uuid, on_get, on_fail) {
  var f_on_get = null;
  var f_on_fail = null;
  if (on_get) {
    f_on_get = function(uuid, field, value) {
      on_get(uuid, value);
    };
  }
  if (on_fail) {
    f_on_fail = function(uuid, field, value) {
      on_fail(uuid);
    };    
  }
  ysid_get_item_field(uuid, "password", f_on_get, f_on_fail);
}

function ysid_get_items(on_get, on_fail) {
  var Y = ysid.Y;
  var od = new Date();
  Y.io("/list/?fields=true",
       {
         on: {
           start: function(id, args) {
             ysid_info("getting items ...");
           },
           success: function(id, o, args) {
             var nd = new Date();
             ysid_info("got items in " + (nd - od)/1000. + " seconds");
             var a = o.responseText.split("\n\n");
             for (var i in a) {
               if (a[i]) {
                 var item = new YsidItem(a[i]);
                 ysid.items[item.uuid] = item;
               }
             }
             on_get();
           },
           failure: function(id, o, args) {
             ysid_error("getting items failed: " + o.responseText);
             if (on_fail) { on_fail(); }
           }
         }
       }
      );
}

function ysid_show_items(items, sorted_field, is_desc) {
  if (items == 1) {
    var result = [];
    ysid.Y.all("#items .item").some(
      function(item) {
        result.push(ysid.items[item.get("id").slice("item".length)]);
        return false;
      });
    items = result;
  }
  if (items == null) items = ysid.items;
  if (typeof items != 'array') {
    var result = [];
    for (var k in items) {
      result.push(items[k]);
    }
    items = result;
  }
  sorted_field = sorted_field || ysid.items_sorted_field || "url";
  if (is_desc == null) is_desc = ysid.items_is_desc;
  ysid.items_sorted_field = sorted_field;
  ysid.items_is_desc = is_desc;
  if (sorted_field) {
    items.sort(
      function(a, b) {
        var r = a[sorted_field] < b[sorted_field] ? -1 : a[sorted_field] > b[sorted_field];
        return r || 0;
      });
    if (is_desc) items.reverse();
  }
  var Y = ysid.Y;
  var list = Y.one("#items");
  var s = "";
  for(var k in items) {
    var item = items[k];
    s += item.to_html();
  }
  list.set("innerHTML", s);
  ysid_set_current_item();
  var nodes = list.all(".item");
  nodes.odd().addClass("item-odd");
  nodes.even().addClass("item-even");
  Y.one("#list_num").set("innerText", "# " + nodes.size());
}

function ysid_list(src) {
  var Y = ysid.Y;
  var filter = Y.one("#list_input").get("value");
  var items = ysid.items;
  if (filter.length) {
    filter = filter.split(' ');
    var filters = [];
    for (var j in filter) {
      filters.push(new YsidItemFilter(filter[j]));
    }
    var result = [];
    for (var k in items) {
      var pass = true;
      var item = items[k];
      for (var i in filters) {
        pass = filters[i].match(item);
        if (!pass) break;
      }
      if (pass) result.push(item);
    }
    items = result;
  }
  ysid_show_items(items, "url");
}

function ysid_init() {
  var Y = ysid.Y;
  Y.on("domready", function() {
         ysid_init_hidden();
         ysid_login_form();
         ysid_edit_form();
         ysid.login_overlay.hide();
         ysid.edit_overlay.hide();
         ysid_get_items(ysid_show_items);
         ysid_init_shortcut();
         ysid_init_menu();
         ysid_show_help();
         ysid_close_help();
         ysid_init_list_sort();
       });
}

function ysid_init_menu() {
  var Y = ysid.Y;
  ysid.menu_item_zero_clip = new ZeroClipboard.Client();
  var clip = ysid.menu_item_zero_clip;
  clip.glue('d_clip_button', 'd_clip_container');
  clip.setHandCursor(true);
  clip.addEventListener(
    "onMouseDown", function() {
      var uuid = ysid_get_current_item_uuid();
      var request = Y.io("/get/" + uuid + "/password/", {sync: true});
      var p;
      if (request.status == 200) {
        p = request.responseText;
      } else {
        ysid_error("get password to copy in sync mode failed: " + o.responseText);
      }
      clip.setText(p);
    });
  clip.addEventListener("onComplete", function() {
                          clip.setText("");
                          ysid.menu_overlay.hide();
                        });
  Y.one("#item-menu").hide();
  Y.delegate(
    "click", function(e) {
      ysid_set_current_item(this);
      if (!ysid.menu_overlay) {
        ysid.menu_overlay = new Y.Overlay(
          {
            srcNode: "#item-menu",
            visible: false
          });
        Y.one("#item-menu").show();
      }
      var overlay = ysid.menu_overlay;
      overlay.set("xy", [e.pageX, e.pageY]);
      overlay.set("constrain", true);
      overlay.render();
      overlay.show();
      var menu = Y.one("#item-menu");
      menu.on(
        "mouseenter", function(e) {
          ysid.menu_enter = true;
        });
      menu.on(
        "mouseleave", function(e) {
          if (ysid.menu_enter) overlay.hide();
          ysid.menu_enter = false;
        });
    }, "#items", ".item");  
}

function ysid_init_hidden() {
  var Y = ysid.Y;
  var hides = Y.all(".isa_hidden");
  hides.hide();
}

function ysid_set_current_item(item) {
  var Y = ysid.Y;
  Y.all(".item_current").removeClass("item_current");
  if (item) item = Y.one(item) || item;
  ysid.current_item = item;
  if (!item) return;
  item.addClass("item_current");
  if (!item.inRegion(item.get("viewportRegion"), true)) item.scrollIntoView();
}

function ysid_get_current_item() {
  return ysid.current_item;
}

function ysid_get_current_item_uuid() {
  var item = ysid.current_item;
  if (item) return item.get("id").slice("item".length);
  return null;
}

function ysid_add_shortcut() {
  var Y = ysid.Y;
  var items = Y.one("#items");
  var body = Y.one("body");
  body.on(
    "shortcut|keypress", function(e) {
      if (ysid.in_input) return;
      ysid_close_help();
      if (e.keyCode == 106 || e.keyCode == 107) { // j,k
        var item = ysid_get_current_item();
        if (!item) {
          if (e.keyCode == 106) item = items.one(".item");
          else item = Y.one("#items > .item:last-child");
        } else {
          if (e.keyCode == 106) item = item.next();
          else item = item.previous();
        }
        ysid_set_current_item(item);
      }
      if (e.keyCode == 47) { // /
        e.preventDefault();
        Y.one("#list_input").focus();
      }
      if (e.keyCode == 97) { // a
        e.preventDefault();
        ysid_edit_form();
      }
      if (e.keyCode == 108) { // l
        e.preventDefault();
        ysid_login_form();
      }
      if (e.keyCode == 104 || e.keyCode == 63 ) { // h, ?
        ysid_show_help();
      }
    });
  body.on(
    "shortcut|keyup", function(e) {
      if (ysid.in_input) return;
      if (e.keyCode == 27) { // escape
        if (ysid.menu_overlay) ysid.menu_overlay.hide();
        if (ysid.help_overlay) ysid.help_overlay.hide();
        if (ysid.login_overlay) ysid.login_overlay.hide();
        if (ysid.edit_overlay) ysid.edit_overlay.hide();
      }
      if (e.keyCode == 46) { // delete
        ysid_menu_item_erase();        
      }
    });
  body.on(
    "shortcut|keypress", function(e) {
      if (ysid.in_input) return;
      if (e.keyCode == 101) { // e
        e.preventDefault();
        ysid_menu_item_edit();
      }
      if (e.keyCode == 116) { // t
        e.preventDefault();
        ysid_menu_item_edit("tags");
      }
      if (e.keyCode == 115) { // s
        ysid_menu_item_show_password();
      }
      if (e.keyCode == 99) { // c
        var item = ysid_get_current_item();
        if (item) {
          Y.io("/copy/" + ysid_get_current_item_uuid() + "/password/",
               {
                 on: {
                   success: function(id, o, args) {
                     ysid_info("copied");
                   },
                   failure: function(id, o, args) {
                     ysid_error("coping failed: "  + o.responseText);
                   }
                 }
               });
        }
      }
    });
  body.on(
    "shortcut|keyup", function(e) {
      if (ysid.in_input) return;
      // ysid_log(e.keyCode);
    });
}

function ysid_remove_shortcut() {
  var Y = ysid.Y;
  var body = Y.one("body");
  body.detach('shortcut|keypress');
}

function ysid_init_shortcut() {
  var Y = ysid.Y;
  ysid_add_shortcut();
  Y.all("input").on(
    "key",
    function(e) {
      Y.all("input").each(function(i) { i.blur(); });
    }, "up:27");
  Y.all("input").on(
    "focus",
    function(e) {
      ysid.in_input = true;// ysid_remove_shortcut();
    });
  Y.all("input").on(
    "blur",
    function (e) {
      ysid.in_input = false; // ysid_add_shortcut();
    }
  );
  Y.one("#list_input").on(
    "key",
    function(e) {
      if (e.keyCode == 13) {
        Y.one("#list_input").blur();
        Y.one("body").simulate("keypress", {charCode: 106} );
      }
    });
}

function ysid_random_it(node, dft) {
var Y = ysid.Y;
  node = Y.one(node) || node;
  dft = dft || "";
  var value = node.get("value");
  value = value || dft;
  if (!value) return;
  var poplen = value.split(' ');
  if (poplen.length != 2) return;
  var pop = poplen[0], len = poplen[1];
  if (!pop || !len) return;
  if (parseInt(len) == NaN) return;
  var pops = new Y.Array(["p", "a", "l", "u", "d", "h", "H", "ad", "ld", "ud"]);
  if (pops.indexOf(pop) == -1) return;
  Y.io("/random/",
       {
         sync: true,
         data: {pop: pop, len: len},
         on: {
           success: function(id, o, args) {
             node.set("value", o.responseText);
           }
         }
       });
}

function ysid_show_help() {
  var Y = ysid.Y;
  if (!ysid.help_overlay) {
    ysid.help_overlay = new Y.Overlay(
      {
        srcNode: "#help",
        visible: false,
        centered: true
      });
  }
  var overlay = ysid.help_overlay;
  overlay.set("centered", true);
  overlay.show();
  overlay.render();
}

function ysid_close_help() {
  if (ysid.help_overlay) {
    ysid.help_overlay.hide();
  }
}

function ysid_init_list_sort() {
  var Y = ysid.Y;
  var nodes = Y.all(".item-field-desc");
  nodes.on(
    "click", function(e) {
      var node = e.target;
      var field = node.get("innerText");
      nodes.removeClass("sort-asc");
      nodes.removeClass("sort-desc");
      if (field == ysid.items_sorted_field) ysid.items_is_desc = !ysid.items_is_desc;
      else ysid.items_is_desc = false;
      if (ysid.items_is_desc) node.addClass("sort-desc");
      else node.addClass("sort-asc");
      ysid.items_sorted_field = field;
      ysid_show_items(1);
    });
}
