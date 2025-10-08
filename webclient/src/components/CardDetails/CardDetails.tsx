// eslint-disable-next-line
import React, { useMemo, useState } from 'react';

import { CardDTO } from 'services';

import Card from '../Card/Card';

import './CardDetails.css';

interface CardProps {
  card: CardDTO;
}

// @TODO: add missing fields (loyalty, hand, etc)

const CardDetails = ({ card }: CardProps) => {
  return (
    <div className='cardDetails'>
      <div className='cardDetails-card'>
        <Card card={card} />
      </div>

      {
        card && (
          <div>
            <div className='cardDetails-attributes'>
              <div className='cardDetails-attribute'>
                <span className='cardDetails-attribute__label'>Name:</span>
                <span className='cardDetails-attribute__value'>{card.name}</span>
              </div>

              {
                (!card.power && !card.toughness) ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>P/T:</span>
                    <span className='cardDetails-attribute__value'>{card.power || 0}/{card.toughness || 0}</span>
                  </div>
                )
              }

              {
                !card.manaCost ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>Cost:</span>
                    <span className='cardDetails-attribute__value'>{card.manaCost.replace(/\{|\}/g, '')}</span>
                  </div>
                )
              }

              {
                !card.convertedManaCost ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>CMC:</span>
                    <span className='cardDetails-attribute__value'>{card.convertedManaCost}</span>
                  </div>
                )
              }

              {
                !card.colorIdentity?.length ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>Identity:</span>
                    <span className='cardDetails-attribute__value'>{card.colorIdentity.join('')}</span>
                  </div>
                )
              }

              {
                !card.colors?.length ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>Color(s):</span>
                    <span className='cardDetails-attribute__value'>{card.colors.join('')}</span>
                  </div>
                )
              }

              {
                !card.types?.length ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>Main Type:</span>
                    <span className='cardDetails-attribute__value'>{card.types.join(', ')}</span>
                  </div>
                )
              }

              {
                !card.type ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>Type:</span>
                    <span className='cardDetails-attribute__value'>{card.type}</span>
                  </div>
                )
              }

              {
                !card.side ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>Side:</span>
                    <span className='cardDetails-attribute__value'>{card.side}</span>
                  </div>
                )
              }

              {
                !card.layout ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>Layout:</span>
                    <span className='cardDetails-attribute__value'>{card.layout}</span>
                  </div>
                )
              }
            </div>

            <div className='cardDetails-text'>
              <div className='cardDetails-text__current'>
                {card.text?.trim()}
              </div>

              <div className='cardDetails-text__flavor'>
                {card.flavorText?.trim()}
              </div>
            </div>
          </div>
        )
      }
    </div>
  );
}

export default CardDetails;
