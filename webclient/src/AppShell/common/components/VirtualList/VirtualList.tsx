// eslint-disable-next-line
import React from "react";

import { FixedSizeList as List } from 'react-window';
import AutoSizer from 'react-virtualized-auto-sizer';

import './VirtualList.css';

const VirtualList = ({ items, itemKey, className = {}, size = 30 }) => (
  <div className="virtual-list">
    <AutoSizer>
      {({ height, width }) => (
        <List
          className={`virtual-list__list ${className}`}
          height={height}
          width={width}
          itemData={items}
          itemCount={items.length}
          itemSize={size}
          itemKey={itemKey}
        >
          {Row}
        </List>
      )}
    </AutoSizer>
  </div>
);

const Row = ({ data, index, style }) => (
  <div style={style}>
    {data[index]}
  </div>
);

export default VirtualList;