const std = @import("std");
const Address = std.net.Address;

pub fn main() !void {
    std.debug.print("Hi!\n", .{});
}

test "convert to network order" {
    const adr_str = "10.11.12.13";
    std.debug.print("\nAddress: {s}\n", .{adr_str});
    const adr = try Address.parseIp4(adr_str, 22);
    std.debug.print("Default: {b:0<32}\n", .{adr.in.sa.addr});
    const nbo = std.mem.nativeToBig(u32, adr.in.sa.addr);
    std.debug.print("ToBigEn: {b:0>32}\n", .{nbo});
    try std.testing.expect(true);
}
