'use strict';

/**
 * Merge two objects together recursively, merging any nested objects as well
 * including arrays and functions.
 *
 * If two key names match and the values aren't functions or arrays, the "latest"
 * key wins (right imporant). If the values are functions or arrays, the values
 * are merged -- either wrapped in a function which calls both functions in a row
 * supplying the final object as the context, or concatenated together with
 * `Array.concat`
 *
 * @method  deepestMerge
 * @param {...object} arguments Objects you'd like to merge together
 * @returns {object} merged object
 */
var deepestMerge = module.exports = function(){
  var args = Array.prototype.slice.call(arguments);
  var dest = {};

  args.forEach(function(src){
    if(typeof src === 'object'){
      Object.keys(src).forEach(function(key){
        var destType = typeof dest[key];
        var srcType = typeof src[key];
        if(Array.isArray(dest[key]) && Array.isArray(src[key])) {
          dest[key] = dest[key].concat(src[key]);
        } else if(destType === 'function' && srcType === 'function'){
          var destFunc = dest[key];
          var srcFunc = src[key];
          var newFunc = function(){
            srcFunc.apply(dest, arguments);
            destFunc.apply(dest, arguments);
          };
          dest[key] = newFunc;
        } else if(destType === 'object' && srcType === 'object'){
          dest[key] = deepestMerge(dest[key], src[key]);
        } else {
          dest[key] = src[key];
        }
      });
    }
  });

  return dest;
};
