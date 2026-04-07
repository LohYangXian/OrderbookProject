## Simplified FIX Protocol for Order Book Project

To introduce more realism into the order book engine, we adopt a simplified FIX-like message format instead of directly passing in-memory order objects.

The protocol preserves the familiar `tag=value` structure used in FIX and retains the standard `8=FIX.4.2` begin string. However, it only supports the minimum fields required by our engine.

### Supported Message Types

- `35=D` — New Order
- `35=G` — Modify Order
- `35=F` — Delete Order

### Supported Tags

- `8` — BeginString
- `35` — MsgType
- `11` — OrderId
- `55` — Symbol
- `54` — Side (`1=Buy`, `2=Sell`)
- `44` — Price
- `38` — Quantity

### Example Messages

New order:

`8=FIX.4.2|35=D|11=1001|55=NVDA|54=1|44=135.50|38=200|`

Modify order:

`8=FIX.4.2|35=G|11=1001|44=136.00|38=150|`

Delete order:

`8=FIX.4.2|35=F|11=1001|`

### Design Rationale

This simplified FIX format makes the project more realistic by modeling how trading systems receive orders in production, while avoiding the full complexity of the official FIX specification. It also provides a fairer basis for performance measurement, since parsing and validation costs are included in benchmarking.