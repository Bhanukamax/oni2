/*
 ClipboardNormalModePasteTest

 Test cases for pasting in normal mode:
 - '*' and '+' registers should always paste from clipboard, regardless of configuration
 - named registers (ie, 'a') should _never_ paste from clipboard, regardless of configuration
 - unnamed register should only paste from clipboard if `["paste"]` is set in configuration
 */

open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module Log = (
  val Log.withNamespace("IntegrationTest.ClipboardNormalModePaste")
);

runTest(
  ~name="InsertMode test - effects batched to runEffects",
  (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  // '*' test case
  setClipboard(Some("abc\n"));

  dispatch(KeyboardInput({isText: true, input: "\""}));
  dispatch(KeyboardInput({isText: true, input: "*"}));
  dispatch(KeyboardInput({isText: true, input: "P"}));
  runEffects();

  wait(~name="Mode switches to insert", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "abc");
    }
  );

  // '*' multi-line test case
  setClipboard(Some("1\n2\n3\n"));

  dispatch(KeyboardInput({isText: true, input: "\""}));
  dispatch(KeyboardInput({isText: true, input: "*"}));
  dispatch(KeyboardInput({isText: true, input: "P"}));
  runEffects();

  wait(~name="Multi-line paste works correctly", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let a = String.equal(line, "1");

      let line = Buffer.getLine(1, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let b = String.equal(line, "2");

      let line = Buffer.getLine(2, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let c = String.equal(line, "3");
      a && b && c;
    }
  );

  // '*' multi-line test case, windows style
  setClipboard(Some("4\r\n5\r\n6\r\n"));

  dispatch(KeyboardInput({isText: true, input: "\""}));
  dispatch(KeyboardInput({isText: true, input: "*"}));
  dispatch(KeyboardInput({isText: true, input: "P"}));
  runEffects();

  wait(~name="Multi-line paste works correctly", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let a = String.equal(line, "4");

      let line = Buffer.getLine(1, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let b = String.equal(line, "5");

      let line = Buffer.getLine(2, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let c = String.equal(line, "6");
      a && b && c;
    }
  );
  // '+' test case
  setClipboard(Some("def\n"));

  dispatch(KeyboardInput({isText: true, input: "\""}));
  dispatch(KeyboardInput({isText: true, input: "*"}));
  dispatch(KeyboardInput({isText: true, input: "P"}));
  runEffects();

  wait(~name="Mode switches to insert", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "def");
    }
  );

  // 'a' test case
  // yank current line - def - to 'a' register
  dispatch(KeyboardInput({isText: true, input: "\""}));
  dispatch(KeyboardInput({isText: true, input: "a"}));
  dispatch(KeyboardInput({isText: true, input: "y"}));
  dispatch(KeyboardInput({isText: true, input: "y"}));
  runEffects();

  setClipboard(Some("ghi\n"));

  dispatch(KeyboardInput({isText: true, input: "\""}));
  dispatch(KeyboardInput({isText: true, input: "a"}));
  dispatch(KeyboardInput({isText: true, input: "P"}));
  runEffects();

  wait(~name="Mode switches to insert", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "def");
    }
  );

  // Test if the configuration is set - paste from unnamed register will pull from the keyboard
  setClipboard(Some("jkl\n"));

  wait(~name="Set configuration to pull clipboard on paste", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          vimUseSystemClipboard: {
            yank: false,
            delete: false,
            paste: true,
          },
        },
      }),
    );
    runEffects();
    true;
  });

  dispatch(KeyboardInput({isText: true, input: "y"}));
  dispatch(KeyboardInput({isText: true, input: "y"}));
  dispatch(KeyboardInput({isText: true, input: "P"}));
  runEffects();

  wait(
    ~name=
      "paste from unnamed register pulls from clipboard when ['paste'] is set",
    (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "jkl");
    }
  );

  // Set configuration back (paste=false), and it should not pull from clipboard
  setClipboard(Some("mno\n"));

  wait(~name="Set configuration to pull clipboard on paste", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          vimUseSystemClipboard: {
            yank: false,
            delete: false,
            paste: false,
          },
        },
      }),
    );
    runEffects();
    true;
  });

  dispatch(KeyboardInput({isText: true, input: "y"}));
  dispatch(KeyboardInput({isText: true, input: "y"}));
  dispatch(KeyboardInput({isText: true, input: "P"}));
  runEffects();

  wait(
    ~name=
      "paste from unnamed register pulls from clipboard when ['paste'] is set",
    (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "jkl");
    }
  );

  // Single line case - should paste in front of previous text
  setClipboard(Some("mno"));
  dispatch(KeyboardInput({isText: true, input: "\""}));
  dispatch(KeyboardInput({isText: true, input: "*"}));
  dispatch(KeyboardInput({isText: true, input: "P"}));
  runEffects();

  wait(~name="paste with single line, from clipboard", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "mnojkl");
    }
  );
});
