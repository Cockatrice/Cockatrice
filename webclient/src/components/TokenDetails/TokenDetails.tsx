// eslint-disable-next-line
import React, { useMemo, useState } from 'react';

import { TokenDTO } from 'services';

import Token from '../Token/Token';

import './TokenDetails.css';

interface TokenProps {
  token: TokenDTO;
}

const TokenDetails = ({ token }: TokenProps) => {
  const props = token?.prop?.value;

  return (
    <div className='tokenDetails'>
      <div className='tokenDetails-token'>
        <Token token={token} />
      </div>

      {
        token && (
          <div>
            <div className='tokenDetails-attributes'>
              <div className='tokenDetails-attribute'>
                <span className='tokenDetails-attribute__label'>Name:</span>
                <span className='tokenDetails-attribute__value'>{token.name?.value}</span>
              </div>

              {
                (!props.pt?.value) ? null : (
                  <div className='tokenDetails-attribute'>
                    <span className='tokenDetails-attribute__label'>P/T:</span>
                    <span className='tokenDetails-attribute__value'>{props.pt.value}</span>
                  </div>
                )
              }

              {
                !props.colors?.value ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>Color(s):</span>
                    <span className='cardDetails-attribute__value'>{props.colors.value}</span>
                  </div>
                )
              }

              {
                !props.maintype?.value ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>Main Type:</span>
                    <span className='cardDetails-attribute__value'>{props.maintype.value}</span>
                  </div>
                )
              }

              {
                !props.type?.value ? null : (
                  <div className='cardDetails-attribute'>
                    <span className='cardDetails-attribute__label'>Type:</span>
                    <span className='cardDetails-attribute__value'>{props.type.value}</span>
                  </div>
                )
              }
            </div>

            {
              !token.text?.value ? null : (
                <div className='tokenDetails-text'>
                  <div className='tokenDetails-text__current'>
                    {token.text.value}
                  </div>
                </div>
              )
            }
          </div>
        )
      }

    </div>
  );
}

export default TokenDetails;
