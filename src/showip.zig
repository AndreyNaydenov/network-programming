const std = @import("std");
const c = @cImport({
    @cInclude("arpa/inet.h");
});

pub fn main() !void {
    const argv = std.os.argv;
    if (argv.len != 2) {
        std.debug.print("usage: showip <hostname>\n", .{});
        std.process.exit(1);
    }

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();

    const name = argv[1][0..std.mem.len(argv[1])];
    const list = std.net.getAddressList(allocator, name, 0) catch |err| {
        std.debug.print("error while resolving address: {}\n", .{err});
        std.process.exit(1);
    };
    defer list.deinit();

    if (list.addrs.len == 0) {
        std.debug.print("couldn't resolve hostname {s}\n", .{name});
    }

    std.debug.print("Resolved addresses:\n", .{});

    // print IP address using C library
    for (list.addrs) |addr| {
        var ipstr: [c.INET6_ADDRSTRLEN:0]u8 = undefined;
        var ipver: [*:0]const u8 = undefined;
        // std.debug.print("{}\n", .{addr});
        if (addr.any.family == std.posix.AF.INET) {
            _ = c.inet_ntop(addr.any.family, &addr.in.sa.addr, &ipstr, c.INET6_ADDRSTRLEN);
            ipver = "IPv4";
        } else {
            _ = c.inet_ntop(addr.any.family, &addr.in6.sa.addr, &ipstr, c.INET6_ADDRSTRLEN);
            ipver = "IPv6";
        }
        std.debug.print("{s}: {s}\n", .{ ipver, @as([*:0]u8, &ipstr) });
    }
}
