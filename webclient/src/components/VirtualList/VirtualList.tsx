// eslint-disable-next-line
import React from "react";

import { List, RowComponentProps } from 'react-window';

import './VirtualList.css';

interface RowData {
  items: any[];
}

const Row = ({ index, style, items }: RowComponentProps<RowData>) => (
  <div style={style}>
    {items[index]}
  </div>
);

const VirtualList = ({ items, className = {}, size = 30 }) => (
  <div className="virtual-list">
    <List<RowData>
      className={`virtual-list__list ${className}`}
      rowCount={items.length}
      rowHeight={size}
      rowComponent={Row}
      rowProps={{ items }}
    />
  </div>
);

export default VirtualList;
