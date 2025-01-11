const std = @import("std");

pub fn build(b: *std.Build) void {
    // take default build options
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // option to specify target source file to build (and run)
    const exe_to_build = b.option([]const u8, "target_exe", "choose executable to build and run");

    if (exe_to_build) |exe_name| {
        // if source file is C set flag
        var use_c = false;
        const extention = std.fs.path.extension(exe_name);
        if (std.mem.eql(u8, extention, ".c")) use_c = true;

        const name = std.fs.path.stem(exe_name);
        const path = std.fs.path.join(b.allocator, &.{ "src/", exe_name }) catch @panic("OOM");

        // exe that we build
        const exe = b.addExecutable(.{
            .name = name,
            .root_source_file = if (!use_c) b.path(path) else null,
            .target = target,
            .optimize = optimize,
        });
        if (use_c) exe.addCSourceFile(.{ .file = b.path(path) });

        // modify install dir to "c" or "zig" depending on a source file extention
        const install = b.addInstallArtifact(exe, .{ .dest_dir = .{ .override = .{ .custom = if (use_c) "c" else "zig" } } });
        b.default_step.dependOn(&install.step);

        // run resulting artifact
        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(b.getInstallStep());
        if (b.args) |args| {
            run_cmd.addArgs(args);
        }
        const run_step = b.step("run", "Run the app");
        run_step.dependOn(&run_cmd.step);
        return;
    }
    // TODO: notify user that he did not provide any build options
    // TODO: add other option that will take all files in "src/" and build them

    // This section isn't used at the moment
    // const exe_unit_tests = b.addTest(.{
    //     .root_source_file = b.path("src/hello.c"),
    //     .target = target,
    //     .optimize = optimize,
    // });

    // const run_exe_unit_tests = b.addRunArtifact(exe_unit_tests);

    // // Similar to creating the run step earlier, this exposes a `test` step to
    // // the `zig build --help` menu, providing a way for the user to request
    // // running the unit tests.
    // const test_step = b.step("test", "Run unit tests");
    // test_step.dependOn(&run_exe_unit_tests.step);
}
